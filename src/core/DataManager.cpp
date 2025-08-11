#include <algorithm>
#include <fstream>

#include "../NeoVSID.h"
#include "DataManager.h"

#if defined(_WIN32)
#include <Windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <cstdlib>
#endif


DataManager::DataManager(vsid::NeoVSID* neoVSID)
	: neoVSID_(neoVSID) {
	aircraftAPI_ = neoVSID_->GetAircraftAPI();
	flightplanAPI_ = neoVSID_->GetFlightplanAPI();
	airportAPI_ = neoVSID_->GetAirportAPI();
	chatAPI_ = neoVSID_->GetChatAPI();
	loggerAPI_ = neoVSID_->GetLogger();
	controllerDataAPI_ = neoVSID_->GetControllerDataAPI();

	configPath_ = getDllDirectory();
	loadAircraftDataJson();
	activeAirports.clear();
}


std::filesystem::path DataManager::getDllDirectory()
{
#if defined(_WIN32)
	PWSTR path = nullptr;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
	std::filesystem::path documentsPath;
	if (SUCCEEDED(hr)) {
		documentsPath = path;
		CoTaskMemFree(path);
	}
	return documentsPath / "NeoRadar/plugins";
#elif defined(__APPLE__) || defined(__linux__)
	const char* homeDir = std::getenv("HOME");
	if (homeDir) {
		return std::filesystem::path(homeDir) / "Documents" / "NeoRadar/plugins";
	}
	return std::filesystem::path(); // Return empty path if HOME is not set
#else
	return std::filesystem::path(); // Return an empty path for unsupported platforms
#endif
}

void DataManager::clearData()
{
	pilots.clear();
	activeAirports.clear();
	configJson_.clear();
	configPath_.clear();
	if (aircraftAPI_)
		aircraftAPI_ = nullptr;
	if (flightplanAPI_)
		flightplanAPI_ = nullptr;
	if (airportAPI_)
		airportAPI_ = nullptr;
}

void DataManager::clearJson()
{
	configJson_.clear();
	aircraftDataJson_.clear();
	rules.clear();
	areas.clear();
}

void DataManager::DisplayMessageFromDataManager(const std::string& message, const std::string& sender)
{
	Chat::ClientTextMessageEvent textMessage;
	textMessage.sentFrom = "NeoVSID";
	(sender.empty()) ? textMessage.message = ": " + message : textMessage.message = sender + ": " + message;
	textMessage.useDedicatedChannel = true;

	chatAPI_->sendClientMessage(textMessage);
}

void DataManager::populateActiveAirports()
{
	std::vector<Airport::AirportConfig> allAirports = airportAPI_->getConfigurations();
	activeAirports.clear();
	rules.clear();
	areas.clear();

	for (const auto& airport : allAirports)
	{
		if (!airport.depRunways.empty())
		{
			activeAirports.push_back(airport.icao);
			parseRules(airport.icao);
			parseAreas(airport.icao);
		}
	}
}

int DataManager::fetchCFL(const Flightplan::Flightplan& flightplan, const std::vector<std::string> activeRules, const std::vector<std::string> activeAreas, const std::string& vsid, bool singleRwy)
{
	std::string oaci = flightplan.origin;
	// Check if configJSON is already the right one, if not, retrieve it
	if (!retrieveCorrectConfigJson(oaci)) {
		loggerAPI_->log(Logger::LogLevel::Warning, "Failed to retrieve config when assigning CFL for: " + oaci);
		return 0;
	}

	std::string sid;
	if (flightplan.route.sid.length() < 3) {
		if (vsid == "") return 0;
		else sid = vsid;
	}
	else {
		sid = flightplan.route.sid;
	}
	
	
	std::string waypoint = sid.substr(0, sid.length() - 2);
	std::string letter = sid.substr(sid.length() - 1, 1);
	nlohmann::ordered_json waypointSidData;

	if (configJson_[oaci]["sids"].contains(waypoint)) {
		waypointSidData = configJson_[oaci]["sids"][waypoint];
	}
	else {
		loggerAPI_->log(Logger::LogLevel::Warning, "SID not found in config for: " + flightplan.callsign + " with SID: " + sid);
		return 0; // SID not found
	}

	if (!waypointSidData.contains(letter)) {
		loggerAPI_->log(Logger::LogLevel::Info, "SID letter not found in waypoint SID data for: " + flightplan.callsign + " when trying to fetch CFL");
		return 0;
	}

	bool ruleActive = !activeRules.empty();
	bool areaActive = !activeAreas.empty();
	auto iterator = waypointSidData[letter].begin();

	while (iterator != waypointSidData[letter].end())
	{
		loggerAPI_->log(Logger::LogLevel::Info, "Checking SID variant: " + iterator.key() + " for flightplan: " + flightplan.callsign);
		std::string variant = iterator.key();

		if ((ruleActive && !waypointSidData[letter][variant].contains("customRule")) || (!ruleActive && waypointSidData[letter][variant].contains("customRule"))) {
			loggerAPI_->log(Logger::LogLevel::Info, "SID has no custom rule but rules are active for: " + flightplan.callsign + " with SID: " + sid + ", variant: " + letter + variant);
			++iterator;
			continue;
		}
		
		if (ruleActive) {
			if (!isMatchingRules(waypointSidData, activeRules, letter, variant)) {
				loggerAPI_->log(Logger::LogLevel::Info, "SID rule not matching for: " + flightplan.callsign + " with SID: " + sid + ", variant: " + letter + variant);
				++iterator;
				continue;
			}
		}

		if (waypointSidData[letter][variant].contains("engineType")) {
			if (!isMatchingEngineRestrictions(waypointSidData[letter][variant], flightplan.acType)) {
				loggerAPI_->log(Logger::LogLevel::Info, "SID engine type restriction not matching for: " + flightplan.callsign + " with SID: " + sid + ", variant: " + letter + variant);
				++iterator;
				continue;
			}
			else {
				loggerAPI_->log(Logger::LogLevel::Info, "SID engine type restriction matches for: " + flightplan.callsign + " with SID: " + sid + ", variant: " + letter + variant);
				return waypointSidData[letter][variant]["initial"].get<int>();
			}
		}
		else {
			loggerAPI_->log(Logger::LogLevel::Info, "SID has no engine type restriction for: " + flightplan.callsign + " with SID: " + sid + ", variant: " + letter + variant);
			return waypointSidData[letter][variant]["initial"].get<int>();
		}
	}
	loggerAPI_->log(Logger::LogLevel::Warning, "No valid CFL found for flightplan: " + flightplan.callsign + " with SID: " + sid);
	return 0; // No valid CFL found
}

sidData DataManager::generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy)
{
	std::string oaci = flightplan.origin;
	std::vector<std::string> activeRules;
	std::vector<std::string> activeAreas;
	std::vector<std::string> aircraftAreas;

	auto airportConfig = airportAPI_->getConfigurationByIcao(oaci);
	if (!airportConfig) {
		loggerAPI_->log(Logger::LogLevel::Warning, "Airport configuration not found for: " + oaci);
		return { depRwy, "CHECKFP", 0};
	}
	std::vector<std::string> depRwys = airportConfig->depRunways;


	bool singleRwy = depRwys.size() < 2;

	for (const auto& rule : this->rules) {
		if (rule.oaci == oaci && rule.active) {
			activeRules.push_back(rule.name);
		}
	}

	for (const auto& area : this->areas) {
		if (area.oaci == oaci && area.active) {
			activeAreas.push_back(area.name);
		}
	}

	std::string suggestedRwy = flightplan.route.suggestedDepRunway;
	if (flightplan.flightRule == "V" || flightplan.route.rawRoute.empty()) {
		loggerAPI_->log(Logger::LogLevel::Warning, "Flightplan has no route or is VFR: " + flightplan.callsign);
		return { suggestedRwy, "------", fetchCFL(flightplan, activeRules, activeAreas, "", singleRwy)};
	}

	std::string firstWaypoint = flightplan.route.waypoints[0].identifier;
	std::string suggestedSid = flightplan.route.suggestedSid;
	if (suggestedSid.empty() || suggestedSid.length() < 2) {
		DisplayMessageFromDataManager("SID not found for waypoint: " + firstWaypoint + " for: " + flightplan.callsign, "SID Assigner");
		loggerAPI_->log(Logger::LogLevel::Warning, "suggested SID length incorrect " + firstWaypoint + " for: " + flightplan.callsign);
		return { suggestedRwy, "CHECKFP", fetchCFL(flightplan, activeRules, activeAreas, "", singleRwy)};
	}
	std::string indicator = suggestedSid.substr(suggestedSid.length() - 2, 1);


	// Check if configJSON is already the right one, if not, retrieve it
	if (!retrieveCorrectConfigJson(oaci)) {
		return { suggestedRwy, suggestedSid, fetchCFL(flightplan, activeRules, activeAreas, "", singleRwy)};
	}

	std::transform(oaci.begin(), oaci.end(), oaci.begin(), ::toupper); //Convert to uppercase
	
	loggerAPI_->log(Logger::LogLevel::Info, "Generating SID for flightplan: " + flightplan.callsign + ", first waypoint: " + firstWaypoint + ", depRwy: " + depRwy);
	
	// Extract waypoint only SID information
	nlohmann::ordered_json waypointSidData;
	if (configJson_.contains(oaci) && configJson_[oaci]["sids"].contains(firstWaypoint)) {
		waypointSidData = configJson_[oaci]["sids"][firstWaypoint];
	} else {
		DisplayMessageFromDataManager("SID not found for waypoint: " + firstWaypoint + " for: " + flightplan.callsign, "SID Assigner");
		loggerAPI_->log(Logger::LogLevel::Warning, "No SID matching firstWaypoint: " + firstWaypoint + " for: " + flightplan.callsign);
		return { suggestedRwy, "CHECKFP", fetchCFL(flightplan, activeRules, activeAreas, "", singleRwy)};
	}


	bool ruleActive = !activeRules.empty();
	bool areaActive = !activeAreas.empty();

	std::optional<Aircraft::Aircraft> aircraft = aircraftAPI_->getByCallsign(flightplan.callsign);

	if (!aircraft.has_value()) return { suggestedRwy, "CHECKFP", 0 };

	double aircraftLat = aircraft->position.latitude;
	double aircraftLon = aircraft->position.longitude;
	std::vector<std::string> areaNames;

	// Get aircraft areas based on its position
	for (const auto& areaName : activeAreas) {
		if (isInArea(aircraftLat, aircraftLon, oaci, areaName)) {
			aircraftAreas.push_back(areaName);
			loggerAPI_->log(Logger::LogLevel::Info, "Aircraft " + flightplan.callsign + " is in area: " + areaName + " for OACI: " + oaci);
		}
	}

	auto sidIterator = waypointSidData.begin();
	while (sidIterator != waypointSidData.end()) {
		std::string sidLetter = sidIterator.key();
		std::string sidRwy = waypointSidData[sidLetter]["1"]["rwy"].get<std::string>();
		if (!waypointSidData[sidLetter]["1"].contains("customRule") && ruleActive) {
			loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " has no custom rule but rules are active for flightplan: " + flightplan.callsign);
			++sidIterator;
			continue;
		}

		bool rwyMatches = false;
		for (const auto& rwy : depRwys) {
			if (sidRwy.find(rwy) != std::string::npos) {
				rwyMatches = true;
				break;
			}
		}
		if (!rwyMatches) {
			loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " does not match any departure runway for flightplan: " + flightplan.callsign);
			++sidIterator;
			continue;
		}

		auto variantIterator = waypointSidData[sidLetter].begin();
		while (variantIterator != waypointSidData[sidLetter].end())
		{
			std::string variant = variantIterator.key();

			if (ruleActive) {
				if (!isMatchingRules(waypointSidData, activeAreas, sidLetter, variant)) {
					++variantIterator;
					continue;
				}
			}
			else {
				if (waypointSidData[sidLetter][variant].contains("customRule")) {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " has a custom rule but no active rules for flightplan: " + flightplan.callsign);
					++variantIterator;
					continue; // Skip this variant if it has a custom rule but no active rules
				}
			}
			
			if (!singleRwy) { // if single runway, we don't check for areas
				if (areaActive) {
					if (!isMatchingAreas(waypointSidData, aircraftAreas, sidLetter, variant, flightplan)) {
						++variantIterator;
						continue; // Skip this variant if it doesn't match active areas
					}
				}
				else {
					if (waypointSidData[sidLetter][variant].contains("area")) {
						loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " has an area restriction but no active areas for flightplan: " + flightplan.callsign);
						++variantIterator;
						continue;
					}
				}
			}

			if (waypointSidData[sidLetter][variant].contains("equip") && waypointSidData[sidLetter][variant]["equip"].contains("RNAV")) {
				if (!isRNAV(flightplan.acType)) {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " requires RNAV but aircraft does not support it for flightplan: " + flightplan.callsign);
					++variantIterator;
					continue; // Skip this variant if RNAV is required but aircraft does not support it
				}
				else {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " matches RNAV for flightplan: " + flightplan.callsign);
				}
			}

			std::string depRwy;
			for (const auto& rwy : depRwys) {
				if (sidRwy.find(rwy) != std::string::npos) {
					depRwy = rwy;
					break;
				}
			}

			if (waypointSidData[sidLetter][variant].contains("engineType")) {
				if (!isMatchingEngineRestrictions(waypointSidData[sidLetter][variant], flightplan.acType)) {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " has an engine type restriction that does not match for flightplan: " + flightplan.callsign);
					++variantIterator;
					continue; // Skip this variant if it doesn't match engine type
				}
				else {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " matches engine type for flightplan: " + flightplan.callsign);
					return { depRwy, firstWaypoint + indicator + sidLetter, fetchCFL(flightplan, activeRules, activeAreas, firstWaypoint + indicator + sidLetter, singleRwy) };
				}
			} 
			else { //No engine restriction
				loggerAPI_->log(Logger::LogLevel::Info, "Assigning SID : " + firstWaypoint + indicator + sidLetter + " for: " + flightplan.callsign);
				return { depRwy, firstWaypoint + indicator + sidLetter, fetchCFL(flightplan, activeRules, activeAreas, firstWaypoint + indicator + sidLetter, singleRwy) };
			}
			++variantIterator; // Fallback
		}
		++sidIterator;
	}
	DisplayMessageFromDataManager("No matching SID found for: " + flightplan.callsign + ", check flighplan, rerouting might be necessary", "SID Assigner");
	loggerAPI_->log(Logger::LogLevel::Warning, "No matching SID found for: " + flightplan.callsign + ", check flightplan, rerouting might be necessary");
	return { suggestedRwy, "CHECKFP", fetchCFL(flightplan, activeRules, activeAreas, "", singleRwy)};
}
	
int DataManager::retrieveConfigJson(const std::string& oaci)
{
	std::string fileName = oaci + ".json";
	std::filesystem::path jsonPath = configPath_ / "NeoVSID" / fileName;

	std::ifstream config(jsonPath);
	if (!config.is_open()) {
		DisplayMessageFromDataManager("Could not open JSON file: " + jsonPath.string(), "DataManager");
		loggerAPI_->log(Logger::LogLevel::Error, "Could not open JSON file: " + jsonPath.string());
		return -1;
	}

	try {
		config >> configJson_;
	}
	catch (...) {
		DisplayMessageFromDataManager("Error parsing JSON file: " + jsonPath.string(), "DataManager");
		loggerAPI_->log(Logger::LogLevel::Error, "Error parsing JSON file: " + jsonPath.string());
		return -1;
	}

	return 0;
}

bool DataManager::retrieveCorrectConfigJson(const std::string& oaci)
{
	if (!configJson_.contains(oaci) || configJson_.empty()) {
		if (retrieveConfigJson(oaci) == -1) {
			DisplayMessageFromDataManager("Error retrieving config JSON for OACI: " + oaci, "DataManager");
			loggerAPI_->log(Logger::LogLevel::Error, "Error retrieving config JSON for OACI: " + oaci);
			return false;
		}
	}
	return true;
}

void DataManager::loadAircraftDataJson()
{
	std::filesystem::path jsonPath = configPath_ / "NeoVSID" / "AircraftData.json";
	std::ifstream aircraftDataFile(jsonPath);
	if (!aircraftDataFile.is_open()) {
		DisplayMessageFromDataManager("Could not open aircraft data JSON file: " + jsonPath.string(), "DataManager");
		loggerAPI_->log(Logger::LogLevel::Error, "Could not open aircraft data JSON file: " + jsonPath.string());
		return;
	}
	try {
		aircraftDataJson_ = nlohmann::json::parse(aircraftDataFile);
	}
	catch (...) {
		DisplayMessageFromDataManager("Error parsing aircraft data JSON file: " + jsonPath.string(), "DataManager");
		loggerAPI_->log(Logger::LogLevel::Error, "Error parsing aircraft data JSON file: " + jsonPath.string());
		return;
	}
}

void DataManager::parseRules(const std::string& oaci)
{
	if (!retrieveCorrectConfigJson(oaci)) {
		return;
	}

	std::string oaciUpper = oaci;
	std::transform(oaciUpper.begin(), oaciUpper.end(), oaciUpper.begin(), ::toupper);

	if (configJson_[oaciUpper].contains("customRules")) {
		loggerAPI_->log(Logger::LogLevel::Info, "Parsing Custom rules from config JSON for OACI: " + oaci);
		auto iterator = configJson_[oaciUpper]["customRules"].begin();
		while (iterator != configJson_[oaciUpper]["customRules"].end()) {
			std::string ruleName = iterator.key();
			// Check if rule already exists in rules vector
			bool alreadyExists = std::any_of(rules.begin(), rules.end(), [&](const ruleData& rule) {
				return rule.oaci == oaci && rule.name == ruleName;
				});
			if (alreadyExists) {
				++iterator;
				continue;
			}
			bool isActive = iterator.value().get<bool>();
			rules.emplace_back(ruleData{ oaci, ruleName, isActive });
			++iterator;
		}
	}
}

void DataManager::parseAreas(const std::string& oaci)
{
	if (!retrieveCorrectConfigJson(oaci)) {
		return;
	}

	std::string oaciUpper = oaci;
	std::transform(oaciUpper.begin(), oaciUpper.end(), oaciUpper.begin(), ::toupper);

	if (configJson_[oaciUpper].contains("areas")) {
		loggerAPI_->log(Logger::LogLevel::Info, "Parsing Areas from config JSON for OACI: " + oaci);
		auto areaIterator = configJson_[oaciUpper]["areas"].begin();
		while (areaIterator != configJson_[oaciUpper]["areas"].end()) {
			std::string areaName = areaIterator.key();

			// Check if area already exists in areas vector
			bool alreadyExists = std::any_of(areas.begin(), areas.end(), [&](const areaData& area) {
				return area.oaci == oaci && area.name == areaName;
				});
			if (alreadyExists) {
				++areaIterator;
				continue;
			}

			std::vector<std::pair<double, double>> waypointsList;
			bool isActive = areaIterator.value()["active"].get<bool>();
			auto waypointIterator = areaIterator.value().begin();
			while (waypointIterator != areaIterator.value().end())
			{
				double lat, lon;
				if (waypointIterator.key() != "active") {
					lat = std::stod(waypointIterator.value()["lat"].get<std::string>());
					lon = std::stod(waypointIterator.value()["lon"].get<std::string>());
					waypointsList.emplace_back(lat, lon);
				}
				++waypointIterator;
			}
			areas.emplace_back(areaData{ oaci, areaName, waypointsList, isActive });
			++areaIterator;
		}
	}
}

Pilot DataManager::getPilotByCallsign(std::string callsign) const
{
	if (callsign.empty())
		return Pilot{};
	for (const auto& pilot : pilots)
	{
		if (pilot.callsign == callsign)
			return pilot;
	}
	return Pilot{};
}

std::vector<std::string> DataManager::getAllDepartureCallsigns() {
	std::vector<PluginSDK::Flightplan::Flightplan> flightplans = flightplanAPI_->getAll();
	std::vector<std::string> callsigns;

	for (const auto& flightplan : flightplans)
	{
		if (flightplan.callsign.empty())
			continue;

		if (!aircraftExists(flightplan.callsign))
			continue;

		if (!isDepartureAirport(flightplan.origin))
			continue;

		std::optional<double> distanceFromOrigin = aircraftAPI_->getDistanceFromOrigin(flightplan.callsign);
		if (!distanceFromOrigin.has_value()) {
			loggerAPI_->log(Logger::LogLevel::Error, "Failed to retrieve distance from origin for callsign: " + flightplan.callsign);
			continue;
		}
		if (distanceFromOrigin > MAX_DISTANCE)
			continue;

		std::optional<PluginSDK::ControllerData::ControllerDataModel> controllerData = controllerDataAPI_->getByCallsign(flightplan.callsign);
		if (!controllerData.has_value()) continue;
		if (controllerData->groundStatus == ControllerData::GroundStatus::Dep)
			continue;

		callsigns.push_back(flightplan.callsign);

		if (pilotExists(flightplan.callsign))
			continue;

		std::string depRwy = flightplan.route.suggestedDepRunway;
		if (flightplan.route.depRunway != "") depRwy = flightplan.route.depRunway;

		sidData vsidData = generateVSID(flightplan, depRwy);
		pilots.push_back(Pilot{ flightplan.callsign, vsidData.rwy, vsidData.sid, vsidData.cfl});
		loggerAPI_->log(Logger::LogLevel::Info, "Added pilot: " + flightplan.callsign + " with SID: " + vsidData.sid + " and CFL: " + std::to_string(vsidData.cfl));
	}
	return callsigns;
}

bool DataManager::isDepartureAirport(const std::string& oaci) const
{
	if (oaci.empty())
		return false;

	for (const auto& airport : activeAirports)
	{
		if (oaci == airport)
			return true;
	}
	return false;
}

bool DataManager::aircraftExists(const std::string& callsign) const
{
	if (callsign.empty())
		return false;
	std::optional<PluginSDK::Aircraft::Aircraft> aircraft = aircraftAPI_->getByCallsign(callsign);
	if (aircraft.has_value())
		return true;
	return false;
}

bool DataManager::pilotExists(const std::string& callsign) const
{
	if (std::find_if(pilots.begin(), pilots.end(), [&](const Pilot& p) { return p.callsign == callsign; }) != pilots.end())
		return true;
	return false;
}

bool DataManager::isInArea(const double& latitude, const double& longitude, const std::string& oaci, const std::string& areaName)
{
	std::vector<double> latitudes, longitudes;

	areaData area = areas.empty() ? areaData{} : *std::find_if(areas.begin(), areas.end(), [&](const areaData& area) {
		return area.oaci == oaci && area.name == areaName;
		});

	if (area.coordinates.empty()) {
		DisplayMessageFromDataManager("Area not found for OACI: " + oaci + ", Area: " + areaName, "DataManager");
		loggerAPI_->log(Logger::LogLevel::Warning, "Area not found for OACI: " + oaci + ", Area: " + areaName);
		return false;
	}

	for (const auto& waypoint : area.coordinates) {
		latitudes.push_back(waypoint.first);
		longitudes.push_back(waypoint.second);
	}

	// Ray casting algorithm for point-in-polygon
	bool inside = false;
	size_t n = latitudes.size();
	if (n < 3) {
		DisplayMessageFromDataManager("Not enough points in area polygon for OACI: " + oaci, "DataManager");
		loggerAPI_->log(Logger::LogLevel::Warning, "Not enough points in area polygon for OACI: " + oaci + ", Area: " + areaName);
		return false;
	}
	for (size_t i = 0, j = n - 1; i < n; j = i++) {
		double xi = latitudes[i], yi = longitudes[i];
		double xj = latitudes[j], yj = longitudes[j];
		bool intersect = ((yi > longitude) != (yj > longitude)) &&
			(latitude < (xj - xi) * (longitude - yi) / (yj - yi + 1e-12) + xi);
		if (intersect)
			inside = !inside;
	}
	return inside;
}

bool DataManager::isMatchingRules(const nlohmann::ordered_json waypointSidData, const std::vector<std::string> activeRules, const std::string& letter, const std::string& variant)
{
	std::vector<std::string> ruleNames;
	if (waypointSidData[letter][variant]["customRule"].is_array()) {
		for (const auto& rule : waypointSidData[letter][variant]["customRule"]) {
			ruleNames.push_back(rule.get<std::string>());
		}
	}
	else {
		ruleNames.push_back(waypointSidData[letter][variant]["customRule"].get<std::string>());
	}

	if (!std::all_of(activeRules.begin(), activeRules.end(), [&](const std::string& activeRuleName) {
		return std::find(ruleNames.begin(), ruleNames.end(), activeRuleName) != ruleNames.end();
		})) {
		return false;
	}
	return true;
}

bool DataManager::isMatchingAreas(const nlohmann::ordered_json waypointSidData, const std::vector<std::string> activeAreas, const std::string& letter, const std::string& variant, const Flightplan::Flightplan fp)
{
	std::vector<std::string> aircraftAreas;

	auto aircraft = aircraftAPI_->getByCallsign(fp.callsign);
	if (!aircraft.has_value()) {
		return false;
	}
	double aircraftLat = aircraft->position.latitude;
	double aircraftLon = aircraft->position.longitude;
	for (const auto& areaName : activeAreas) {
		if (isInArea(aircraftLat, aircraftLon, fp.origin, areaName)) {
			aircraftAreas.push_back(areaName);
		}
	}
	std::vector<std::string> areaNames;
	if (waypointSidData[letter][variant].contains("area")) {
		if (waypointSidData[letter][variant]["area"].is_array()) {
			for (const auto& area : waypointSidData[letter][variant]["area"]) {
				areaNames.push_back(area.get<std::string>());
			}
		}
		else {
			areaNames.push_back(waypointSidData[letter][variant]["area"].get<std::string>());
		}
		if (!std::all_of(aircraftAreas.begin(), aircraftAreas.end(), [&](const std::string& activeAreaName) {
			return std::find(areaNames.begin(), areaNames.end(), activeAreaName) != areaNames.end();
			})) {
			return false;
		}
	}
	return true;
}

bool DataManager::isMatchingEngineRestrictions(const nlohmann::ordered_json sidData, const std::string& aircraftType)
{
	std::string engineType = "J"; // Defaulting to Jet if no type is found
	std::string requiredEngineType = "J";

	if (aircraftDataJson_.contains(aircraftType))
		engineType = aircraftDataJson_[aircraftType]["engineType"].get<std::string>();

	requiredEngineType = sidData["engineType"].get<std::string>();
	if (requiredEngineType.find(engineType) != std::string::npos) {
		return true;
	}
	return false;
}

bool DataManager::isRNAV(const std::string& aircraftType)
{
	if (aircraftDataJson_.contains(aircraftType)) {
		if (aircraftDataJson_[aircraftType].contains("rnav")) {
			return aircraftDataJson_[aircraftType]["rnav"].get<bool>();
		}
		else {
			loggerAPI_->log(Logger::LogLevel::Warning, "RNAV data not found for aircraft type: " + aircraftType);
			return false;
		}
	}
}

void DataManager::switchRuleState(const std::string& oaci, const std::string& ruleName)
{
	if (oaci.empty() || ruleName.empty())
		return;
	auto it = std::find_if(rules.begin(), rules.end(), [&](const ruleData& rule) {
		return rule.oaci == oaci && rule.name == ruleName;
	});
	if (it != rules.end()) {
		it->active = !it->active;
	}
	else {
		loggerAPI_->log(Logger::LogLevel::Warning, "Rule not found when trying to switch state: " + ruleName + " for OACI: " + oaci);
	}
	removeAllPilots();
}

void DataManager::switchAreaState(const std::string& oaci, const std::string& areaName)
{
	if (oaci.empty() || areaName.empty())
		return;
	auto it = std::find_if(areas.begin(), areas.end(), [&](const areaData& area) {
		return area.oaci == oaci && area.name == areaName;
	});
	if (it != areas.end()) {
		it->active = !it->active;
	}
	else {
		loggerAPI_->log(Logger::LogLevel::Warning, "Area not found when trying to switch state: " + areaName + " for OACI: " + oaci);
	}
	removeAllPilots();
}

void DataManager::addPilot(const std::string& callsign)
{
	auto flightplan = flightplanAPI_->getByCallsign(callsign);

	if (!flightplan.has_value()) {
		return;
	}

	if (callsign.empty())
		return;
	if (pilotExists(callsign))
		return;
	if (!isDepartureAirport(flightplan->origin))
		return;
	std::string depRwy = flightplan->route.suggestedDepRunway;
	if (flightplan->route.depRunway != "")
		depRwy = flightplan->route.depRunway;

	sidData vsidData = generateVSID(flightplan.value(), depRwy);
	pilots.push_back(Pilot{ flightplan->callsign, vsidData.rwy, vsidData.sid, vsidData.cfl });
}

bool DataManager::removePilot(const std::string& callsign)
{
	if (callsign.empty())
		return false;
	auto previousSize = pilots.size();
	pilots.erase(std::remove_if(pilots.begin(), pilots.end(), [&](const Pilot& p) { return p.callsign == callsign; }), pilots.end());
	if (previousSize == pilots.size() && previousSize == 0)
		return false; // No pilot was removed
	return true;
}

void DataManager::removeAllPilots()
{
	pilots.clear();
}
