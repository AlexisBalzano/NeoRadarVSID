#include <fstream>
#include <algorithm>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

#include "DataManager.h"

DataManager::DataManager(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI)
    : aircraftAPI_(aircraftAPI), flightplanAPI_(flightplanAPI), airportAPI_(airportAPI) {
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
    std::string waypoint = sid.substr(0, sid.length() - 2);
    std::string letter = sid.substr(sid.length() - 1, 1);

    if (configJson_.contains(oaci) && configJson_[oaci]["sids"].contains(waypoint) && configJson_[oaci]["sids"][waypoint].contains(letter)) {
        return configJson_[oaci]["sids"][waypoint][letter]["1"]["initial"].get<int>();
    }

    return -1;
}

int DataManager::retrieveConfigJson(const std::string& oaci)
{
    std::string fileName = oaci + ".json";
    std::filesystem::path jsonPath = configPath_ / fileName;

    std::ifstream config(jsonPath);
    if (!config.is_open()) {
        //TODO: displaying of error messages
        return -1;
    }

    try {
        config >> configJson_;
    }
    catch (...) {
        return -1;
    }
	return 0; // Return 0 if the JSON file was successfully retrieved
}

Pilot DataManager::getPilotByCallsign(std::string callsign) const
{
	if (callsign.empty())
		return Pilot{}; // Return an empty Pilot object if the callsign is empty
    for (auto pilot : pilots)
    {
        if (pilot.callsign == callsign)
			return pilot;
    }
    return Pilot{};
}

std::vector<std::string> DataManager::getAllDepartureCallsigns() {
    std::vector<PluginSDK::Flightplan::Flightplan> flightplans = flightplanAPI_->getAll();
    std::vector<std::string> callsigns;

	pilots.clear(); // Clear the pilots vector to ensure it starts fresh
    
    for (const auto& flightplan : flightplans)
    {
        if (flightplan.callsign.empty())
            continue;

        // Ensure the aircraft exists
        if (!pilotExists(flightplan.callsign))
            continue;

		// Ensure the flightplan is from a departure airport
        if (!isDepartureAirport(flightplan.origin))
			continue;
    
        // Ensure the aircraft is still at its departure airport
        if (aircraftAPI_->getDistanceFromOrigin(flightplan.callsign) > 4.)
            continue;
    
        // Check if the callsign is already in the vector
        if (std::find(callsigns.begin(), callsigns.end(), flightplan.callsign) == callsigns.end())
        {
            callsigns.push_back(flightplan.callsign);
            int vsidCfl = fetchCFLfromJson(flightplan);
			pilots.push_back(Pilot{ flightplan.callsign, flightplan.route.suggestedDepRunway, flightplan.route.suggestedSid, vsidCfl }); // Create a new Pilot object
        }
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

bool DataManager::pilotExists(const std::string& callsign) const
{
    if (callsign.empty())
		return false;
    std::optional<PluginSDK::Aircraft::Aircraft> aircraft = aircraftAPI_->getByCallsign(callsign);
	if (aircraft.has_value())
		return true;
    return false;
}

//DELETE: after testing
void DataManager::addPilot(const std::string& callsign)
{
    Flightplan::Flightplan flightplan = flightplanAPI_->getByCallsign(callsign).value();
    
    if (callsign.empty())
        return;
    // Check if the pilot already exists
    if (std::find_if(pilots.begin(), pilots.end(), [&](const Pilot& p) { return p.callsign == callsign; }) != pilots.end())
        return;
    // Add the new pilot
    pilots.push_back(Pilot{ flightplan.callsign, flightplan.route.suggestedDepRunway, flightplan.route.suggestedSid, fetchCFLfromJson(flightplan) });

}
