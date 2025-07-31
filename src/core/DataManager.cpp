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
		if (airport.status == Airport::AirportStatus::Active)
		{
			activeAirports.push_back(airport.icao);
			parseRules(airport.icao);
			parseAreas(airport.icao);
		}
	}
}

int DataManager::fetchCFL(const Flightplan::Flightplan& flightplan, const std::vector<std::string> activeRules, const std::string& vsid)
{
	//Config should already be the correct one since loaded in generateVSID(), check not needed
	std::string sid;
	if (flightplan.route.sid.length() < 3) {
		if (vsid == "") return 0;
		else sid = vsid;
	}
	
	std::string oaci = flightplan.origin;
	std::string waypoint = sid.substr(0, sid.length() - 2);
	std::string letter = sid.substr(sid.length() - 1, 1);
	nlohmann::ordered_json waypointSidData;

	if (configJson_.contains(oaci) && configJson_[oaci]["sids"].contains(waypoint)) {
		waypointSidData = configJson_[oaci]["sids"][waypoint];
	}
	if (!waypointSidData.contains(letter)) {
		loggerAPI_->log(Logger::LogLevel::Warning, "SID letter not found in waypoint SID data for: " + flightplan.callsign + " when trying to fetch CFL");
		return 0;
	}

	bool ruleActive = !activeRules.empty();
	auto iterator = waypointSidData[letter].begin();

	while (iterator != waypointSidData[letter].end())
	{
		std::string variant = iterator.key();
		loggerAPI_->log(Logger::LogLevel::Info, "Checking variant: " + variant + " for flightplan: " + flightplan.callsign + ", SID: " + sid);

		if ((ruleActive && !waypointSidData[letter][variant].contains("customRule")) || (!ruleActive && waypointSidData[letter][variant].contains("customRule"))) {
			++iterator;
			continue;
		}
		if (ruleActive) {
			bool ruleMatch = false;
			for (const auto& rule : activeRules) {
				if (waypointSidData[letter][variant]["customRule"].get<std::string>() == rule) {
					ruleMatch = true;
					break;
				}
			}
			if (!ruleMatch) {
				++iterator;
				continue;
			}
		}

		if (waypointSidData[letter][variant].contains("engineType")) {
			std::string aircraftType = flightplan.acType;
			std::string engineType = "J"; // Defaulting to Jet if no type is found
			std::string requiredEngineType = "J";
	
			if (aircraftDataJson_.contains(aircraftType))
				engineType = aircraftDataJson_[aircraftType]["engineType"].get<std::string>();

			requiredEngineType = waypointSidData[letter][variant]["engineType"].get<std::string>();
			if (requiredEngineType.find(engineType) != std::string::npos) {
				return waypointSidData[letter][variant]["initial"].get<int>();
			}
			else { // Engine type doesn't match
				++iterator;
				continue;
			}
		}
		else {
			return waypointSidData[letter][variant]["initial"].get<int>();
		}
	}
	loggerAPI_->log(Logger::LogLevel::Warning, "No valid CFL found for flightplan: " + flightplan.callsign + " with SID: " + sid);
	return 0; // No valid CFL found
}

sidData DataManager::generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy)
{
	std::string oaci = flightplan.origin;
	std::string aircraftType = flightplan.acType;
	std::string engineType = "J"; // Defaulting to Jet if no type is found
	std::string requiredEngineType = "J";
	std::vector<std::string> activeRules;
	std::vector<std::string> activeAreas;
	std::vector<std::string> aircraftAreas;
	std::vector<std::string> depRwys = airportAPI_->getConfigurationByIcao(oaci)->depRunways;

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
		return { suggestedRwy, "------", fetchCFL(flightplan, activeRules, "")};
	}

	std::string firstWaypoint = flightplan.route.waypoints[0].identifier;
	std::string suggestedSid = flightplan.route.suggestedSid;
	if (suggestedSid.empty() || suggestedSid.length() < 2) {
		DisplayMessageFromDataManager("SID not found for waypoint: " + firstWaypoint + " for: " + flightplan.callsign, "SID Assigner");
		loggerAPI_->log(Logger::LogLevel::Warning, "suggested SID length incorrect " + firstWaypoint + " for: " + flightplan.callsign);
		return { suggestedRwy, "CHECKFP", fetchCFL(flightplan, activeRules, "")};
	}
	std::string indicator = suggestedSid.substr(suggestedSid.length() - 2, 1);


	// Check if configJSON is already the right one, if not, retrieve it
	if (!retrieveCorrectConfigJson(oaci)) {
		return { suggestedRwy, suggestedSid, fetchCFL(flightplan, activeRules, "")};
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
		return { suggestedRwy, "CHECKFP", fetchCFL(flightplan, activeRules, "")};
	}


	bool ruleActive = !activeRules.empty();
	bool areaActive = !activeAreas.empty();

	double aircraftLat = aircraftAPI_->getByCallsign(flightplan.callsign)->position.latitude;
	double aircraftLon = aircraftAPI_->getByCallsign(flightplan.callsign)->position.longitude;
	std::vector<std::string> areaNames;

	// Get aircraft areas based on its position
	for (const auto& areaName : activeAreas) {
		if (isInArea(aircraftLat, aircraftLon, oaci, areaName)) {
			aircraftAreas.push_back(areaName);
			loggerAPI_->log(Logger::LogLevel::Info, "Aircraft " + flightplan.callsign + " is in area: " + areaName + " for OACI: " + oaci);
		}
	}

	if (aircraftDataJson_.contains(aircraftType))
		engineType = aircraftDataJson_[aircraftType]["engineType"].get<std::string>();

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

			std::vector<std::string> ruleNames;
			if (ruleActive) {
				if (waypointSidData[sidLetter][variant]["customRule"].is_array()) {
					for (const auto& rule : waypointSidData[sidLetter][variant]["customRule"]) {
						ruleNames.push_back(rule.get<std::string>());
						loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " rule: " + rule.get<std::string>() + " for flightplan: " + flightplan.callsign);
					}
				}
				else {
					ruleNames.push_back(waypointSidData[sidLetter][variant]["customRule"].get<std::string>());
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " rule: " + ruleNames[0] + " for flightplan: " + flightplan.callsign);
				}

				if(!std::all_of(activeRules.begin(), activeRules.end(), [&](const std::string& activeRuleName) {
					return std::find(ruleNames.begin(), ruleNames.end(), activeRuleName) != ruleNames.end();
				})) {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " does not match all active rules for flightplan: " + flightplan.callsign);
					++variantIterator;
					continue; // Skip this variant if not matching all active rules
				}
			}
			else {
				if (waypointSidData[sidLetter][variant].contains("customRule")) {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " has a custom rule but no active rules for flightplan: " + flightplan.callsign);
					++variantIterator;
					continue; // Skip this variant if it has a custom rule but no active rules
				}
			}
			
			if (areaActive) {
				if (waypointSidData[sidLetter][variant].contains("area")) {
					if (waypointSidData[sidLetter][variant]["area"].is_array()) {
						for (const auto& area : waypointSidData[sidLetter][variant]["area"]) {
							areaNames.push_back(area.get<std::string>());
						}
					}
					else {
						areaNames.push_back(waypointSidData[sidLetter][variant]["area"].get<std::string>());
					}
					if (!std::all_of(aircraftAreas.begin(), aircraftAreas.end(), [&](const std::string& activeAreaName) {
						return std::find(areaNames.begin(), areaNames.end(), activeAreaName) != areaNames.end();
						})) {
						loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " does not match all active areas for flightplan: " + flightplan.callsign);
						++variantIterator;
						continue; // Skip this variant if not matching all areas
					}
				}
			}
			else {
				if (waypointSidData[sidLetter][variant].contains("area")) {
					loggerAPI_->log(Logger::LogLevel::Info, "SID " + firstWaypoint + indicator + sidLetter + " has an area restriction but no active areas for flightplan: " + flightplan.callsign);
					++variantIterator;
					continue;
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
				requiredEngineType = waypointSidData[sidLetter][variant]["engineType"].get<std::string>();
				if (requiredEngineType.find(engineType) != std::string::npos) {
					// Engine type matches, we can assign this SID and CFL
					loggerAPI_->log(Logger::LogLevel::Info, "Assigning SID with engineType restriction : " + firstWaypoint + indicator + sidLetter + " for: " + flightplan.callsign);
					return { depRwy, firstWaypoint + indicator + sidLetter, fetchCFL(flightplan, activeRules, firstWaypoint + indicator + sidLetter) };
				}
				else { // Engine type doesn't match
					loggerAPI_->log(Logger::LogLevel::Info, "Engine type mismatch for: " + flightplan.callsign + ", on: " + firstWaypoint + indicator + sidLetter);
					++variantIterator;
					continue;
				}
			} 
			else { //No engine restriction
				loggerAPI_->log(Logger::LogLevel::Info, "Assigning SID : " + firstWaypoint + indicator + sidLetter + " for: " + flightplan.callsign);
				return { depRwy, firstWaypoint + indicator + sidLetter, fetchCFL(flightplan, activeRules, firstWaypoint + indicator + sidLetter) };
			}
			++variantIterator; // Fallback
		}
		++sidIterator;
	}
	DisplayMessageFromDataManager("No matching SID found for: " + flightplan.callsign + ", check flighplan, rerouting might be necessary", "SID Assigner");
	loggerAPI_->log(Logger::LogLevel::Warning, "No matching SID found for: " + flightplan.callsign + ", check flightplan, rerouting might be necessary");
	return { suggestedRwy, "CHECKFP", fetchCFL(flightplan, activeRules, "")};
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

		if (aircraftAPI_->getDistanceFromOrigin(flightplan.callsign) > MAX_DISTANCE)
			continue;

		if (controllerDataAPI_->getByCallsign(flightplan.callsign)->groundStatus == ControllerData::GroundStatus::Dep)
			continue;

		callsigns.push_back(flightplan.callsign);

		if (pilotExists(flightplan.callsign))
			continue;

		sidData vsidData = generateVSID(flightplan, flightplan.route.suggestedDepRunway);
		pilots.push_back(Pilot{ flightplan.callsign, vsidData.rwy, vsidData.sid, vsidData.cfl});
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
	Flightplan::Flightplan flightplan = flightplanAPI_->getByCallsign(callsign).value();

	if (callsign.empty())
		return;
	if (pilotExists(callsign))
		return;
	if (!isDepartureAirport(flightplan.origin))
		return;

	sidData vsidData = generateVSID(flightplan, flightplan.route.suggestedDepRunway);
	pilots.push_back(Pilot{ flightplan.callsign, vsidData.rwy, vsidData.sid, vsidData.cfl });
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
