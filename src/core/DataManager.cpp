#include "DataManager.h"

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

		// Ensure the flightplan is from a departure airport
        if (!isDepartureAirport(flightplan.origin))
			continue;
    
        // Ensure the aircraft exists
        if (!pilotExists(flightplan.callsign))
            continue;
    
        // Ensure the aircraft is still at its departure airport
        if (aircraftAPI_->getDistanceFromOrigin(flightplan.callsign) > 4.)
            continue;
    
        // Check if the callsign is already in the vector
        if (std::find(callsigns.begin(), callsigns.end(), flightplan.callsign) == callsigns.end())
        {
            callsigns.push_back(flightplan.callsign);
			pilots.push_back(Pilot{ flightplan.callsign, "---", "------", 13000}); // Create a new Pilot object with default values
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
