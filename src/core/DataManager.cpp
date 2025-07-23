#include <fstream>
#include <algorithm>

#include "../NeoVSID.h"
#include "DataManager.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif


DataManager::DataManager(vsid::NeoVSID* neoVSID)
	: neoVSID_(neoVSID) {
	aircraftAPI_ = neoVSID_->GetAircraftAPI();
	flightplanAPI_ = neoVSID_->GetFlightplanAPI();
	airportAPI_ = neoVSID_->GetAirportAPI();
	chatAPI_ = neoVSID_->GetChatAPI();
	controllerDataAPI_ = neoVSID_->GetControllerDataAPI();

	configPath_ = getDllDirectory();
	loadAircraftDataJson();
	activeAirports.clear();
}


std::filesystem::path DataManager::getDllDirectory()
{
#if defined(_WIN32)
	wchar_t buffer[MAX_PATH];
	HMODULE hModule = nullptr;
	// Use the address of this function to get the module handle
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&getDllDirectory), &hModule);
	GetModuleFileNameW(hModule, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__)
	Dl_info dl_info;
	dladdr((void*)&getDllDirectory, &dl_info);
	return std::filesystem::path(dl_info.dli_fname).parent_path();
#elif defined(__linux__)
	Dl_info dl_info;
	dladdr((void*)&getDllDirectory, &dl_info);
	return std::filesystem::path(dl_info.dli_fname).parent_path();
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

	for (const auto& airport : allAirports)
	{
		if (airport.status == Airport::AirportStatus::Active)
		{
			activeAirports.push_back(airport.icao);
		}
	}
}

std::string DataManager::generateVRWY(const Flightplan::Flightplan& flightplan)
{
	/* TODO:
	* - generer la piste en fonction des pistes de departs actuellement configurées sur le terrain de départ du trafic
	* - generer la piste en fonction de la position du trafic par rapport au terrain de départ si plusieurs pistes de départ et config roulage mini
	* - generer la piste en fonction de la direction du depart si config croisement au sol
	*/ 
	return flightplan.route.suggestedDepRunway;
}

sidData DataManager::generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy)
{
	/* TODO:
	* - generer SID en fonction de la piste assignée
	* - tester implementation actuelle
	*/
	if (flightplan.flightRule == "V" || flightplan.route.rawRoute.empty()) {
		return { "------", 0 };
	}

	std::string suggestedSid = flightplan.route.suggestedSid;
	std::string firstWaypoint = flightplan.route.waypoints[0].identifier;

	// Check if configJSON is already the right one, if not, retrieve it
	std::string oaci = flightplan.origin;
	if (!configJson_.contains(oaci) || configJson_.empty()) {
		if (retrieveConfigJson(oaci) == -1) {
			DisplayMessageFromDataManager("Error retrieving config JSON for OACI: " + oaci, "DataManager");
			return { suggestedSid, 0};
		}
	}

	// Extract waypoint only SID information
	std::transform(oaci.begin(), oaci.end(), oaci.begin(), ::toupper); //Convert to uppercase
	if (suggestedSid.empty() || suggestedSid.length() < 2) {
		return { suggestedSid, 0};
	}
	std::string waypoint = suggestedSid.substr(0, suggestedSid.length() - 2);
	std::string indicator = suggestedSid.substr(suggestedSid.length() - 2, 1);
	std::string letter = suggestedSid.substr(suggestedSid.length() - 1, 1);

	nlohmann::json waypointSidData;

	if (configJson_.contains(oaci) && configJson_[oaci]["sids"].contains(firstWaypoint)) {
		waypointSidData = configJson_[oaci]["sids"][firstWaypoint];
	} else {
		DisplayMessageFromDataManager("SID not found in config JSON for waypoint: " + firstWaypoint + " for: " + flightplan.callsign, "DataManager");
		return { suggestedSid, 0};
	}

	//TODO: refactor using iterator for all cases to make it more readable
	//From here we have all the data necessary from oaci.json
	if (waypointSidData.contains(letter)) {
		if (waypointSidData[letter]["1"].contains("engineType")) {
			std::string aircraftType = flightplan.acType;
			std::string engineType = "J"; // Defaulting to Jet if no type is found
			if (aircraftDataJson_.contains(aircraftType))
				engineType = aircraftDataJson_[aircraftType]["engineType"].get<std::string>();
			std::string requiredEngineType = waypointSidData[letter]["1"]["engineType"].get<std::string>();
			if (requiredEngineType.find(engineType) != std::string::npos) {
				// Engine type matches, we can assign this SID and CFL
				int fetchedCfl = waypointSidData[letter]["1"]["initial"].get<int>();
				return { firstWaypoint + indicator + letter, fetchedCfl };
			}
			else {
				if (waypointSidData.contains("2")) {
					// If there is a CFL for the other engine type, we assign it
					int fetchedCfl = waypointSidData[letter]["2"]["initial"].get<int>();
					return { firstWaypoint + indicator + letter, fetchedCfl };
				}
				else {
					// Iterate over the next SIDs for the same runway to find the other engine type one
					auto it = waypointSidData.begin();
					while (it != waypointSidData.end()) {
						std::string nextLetter = it.key();
						std::string sidRwy = waypointSidData[nextLetter]["1"]["rwy"].get<std::string>();
						if (sidRwy.find(depRwy) != std::string::npos) {
							if (waypointSidData[nextLetter]["1"].contains("engineType")) {
								requiredEngineType = waypointSidData[nextLetter]["1"]["engineType"].get<std::string>();
								if (requiredEngineType.find(engineType) != std::string::npos) {
									// Engine type matches, we can assign this SID and CFL
									int fetchedCfl = waypointSidData[letter]["1"]["initial"].get<int>();
									return { firstWaypoint + indicator + nextLetter, fetchedCfl };
								}
							}
						}
						++it;
					}
					// If no next letter, we return the first SID with its CFL
					DisplayMessageFromDataManager("No matching engine type SID found for: " + flightplan.callsign, "DataManager");
					return { suggestedSid, waypointSidData[letter]["1"]["initial"].get<int>() };
				}

			}
		}
		else {
			// If no engine restriction then we assign this SID and CFL
			int fetchedCfl = waypointSidData[letter]["1"]["initial"].get<int>();
			return { firstWaypoint + indicator + letter, fetchedCfl };
		}
	}
	return { suggestedSid, 0 };
}

int DataManager::retrieveConfigJson(const std::string& oaci)
{
	std::string fileName = oaci + ".json";
	std::filesystem::path jsonPath = configPath_ / "NeoVSID" / fileName;

	std::ifstream config(jsonPath);
	if (!config.is_open()) {
		DisplayMessageFromDataManager("Could not open JSON file: " + jsonPath.string(), "DataManager");
		return -1;
	}

	try {
		config >> configJson_;
	}
	catch (...) {
		DisplayMessageFromDataManager("Error parsing JSON file: " + jsonPath.string(), "DataManager");
		return -1;
	}
	return 0; // Return 0 if the JSON file was successfully retrieved
}

void DataManager::loadAircraftDataJson()
{
	std::filesystem::path jsonPath = configPath_ / "NeoVSID" / "AircraftData.json";
	std::ifstream aircraftDataFile(jsonPath);
	if (!aircraftDataFile.is_open()) {
		DisplayMessageFromDataManager("Could not open aircraft data JSON file: " + jsonPath.string(), "DataManager");
		return;
	}
	try {
		aircraftDataJson_ = nlohmann::json::parse(aircraftDataFile);
	}
	catch (...) {
		DisplayMessageFromDataManager("Error parsing aircraft data JSON file: " + jsonPath.string(), "DataManager");
		return;
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

		std::string vsidRwy = generateVRWY(flightplan);
		sidData vsidData = generateVSID(flightplan, vsidRwy);
		pilots.push_back(Pilot{ flightplan.callsign, vsidRwy, vsidData.sid, vsidData.cfl});
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

void DataManager::addPilot(const std::string& callsign)
{
	Flightplan::Flightplan flightplan = flightplanAPI_->getByCallsign(callsign).value();

	if (callsign.empty())
		return;
	if (pilotExists(callsign))
		return;
	if (!isDepartureAirport(flightplan.origin))
		return;

	std::string vsidRwy = generateVRWY(flightplan);
	sidData vsidData = generateVSID(flightplan, vsidRwy);
	pilots.push_back(Pilot{ flightplan.callsign, vsidRwy, vsidData.sid, vsidData.cfl });
}

bool DataManager::removePilot(const std::string& callsign)
{
	if (callsign.empty())
		return false;
	auto previousSize = pilots.size();
	pilots.erase(std::remove_if(pilots.begin(), pilots.end(), [&](const Pilot& p) { return p.callsign == callsign; }), pilots.end());
	if (previousSize == pilots.size() && previousSize != 0)
		return false; // No pilot was removed
	return true;
}

void DataManager::removeAllPilots()
{
	pilots.clear();
}
