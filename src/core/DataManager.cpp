#include <fstream>
#include <algorithm>

#include "../NeoVSID.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

#include "DataManager.h"

DataManager::DataManager(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI, Chat::ChatAPI* chatAPI)
	: aircraftAPI_(aircraftAPI), flightplanAPI_(flightplanAPI), airportAPI_(airportAPI), chatAPI_(chatAPI) {
	configPath_ = getDllDirectory();
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

	for (const auto& airport : allAirports)
	{
		// Check if the airport is active
		if (airport.status == Airport::AirportStatus::Active)
		{
			activeAirports.push_back(airport.icao);
		}
	}
}

int DataManager::fetchCFLfromJson(const Flightplan::Flightplan& flightplan)
{
	std::string oaci = flightplan.origin;
	if (!configJson_.contains(oaci) || configJson_.empty()) {
		if (retrieveConfigJson(oaci) == -1) {
			return -1; // Return -1 if the JSON file could not be retrieved
		}
	}

	std::transform(oaci.begin(), oaci.end(), oaci.begin(), ::toupper); //Convert to uppercase
	std::string sid = flightplan.route.suggestedSid;
	if (sid.empty() || sid.length() < 2) {
		return -1; // Return -1 if the SID is empty or too short
	}
	std::string waypoint = sid.substr(0, sid.length() - 2);
	std::string letter = sid.substr(sid.length() - 1, 1);

	if (configJson_.contains(oaci) && configJson_[oaci]["sids"].contains(waypoint) && configJson_[oaci]["sids"][waypoint].contains(letter)) {
		return configJson_[oaci]["sids"][waypoint][letter]["1"]["initial"].get<int>();
	}

	return -1;
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

std::string DataManager::generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy)
{
	/* TODO:
	* - generer SID en fonction de la piste assignée
	* - generer SID en fonction du type de moteur du trafic
	* - recuperer le CFL associé à la SID générée
	*/
	return flightplan.route.suggestedSid;
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

	pilots.clear();

	for (const auto& flightplan : flightplans)
	{
		if (flightplan.callsign.empty())
			continue;

		if (!aircraftExists(flightplan.callsign))
			continue;

		if (!isDepartureAirport(flightplan.origin))
			continue;

		if (aircraftAPI_->getDistanceFromOrigin(flightplan.callsign) > 4.)
			continue;

		callsigns.push_back(flightplan.callsign);

		std::string vsidRwy = generateVRWY(flightplan);
		pilots.push_back(Pilot{ flightplan.callsign, vsidRwy, generateVSID(flightplan, vsidRwy), fetchCFLfromJson(flightplan) });
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
	// Check if the pilot already exists
	if (std::find_if(pilots.begin(), pilots.end(), [&](const Pilot& p) { return p.callsign == callsign; }) != pilots.end())
		return;

	pilots.push_back(Pilot{ flightplan.callsign, flightplan.route.suggestedDepRunway, flightplan.route.suggestedSid, fetchCFLfromJson(flightplan) });
}