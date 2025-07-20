#pragma once
#include <vector>

#include "SDK.h"

using namespace PluginSDK;
class DataManager {
public:
	DataManager(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI) : aircraftAPI_(aircraftAPI), flightplanAPI_(flightplanAPI), airportAPI_(airportAPI) {}
	~DataManager() = default;

	
	static DataManager* instance(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI) {
		static DataManager instance(aircraftAPI, flightplanAPI, airportAPI);
		return &instance;
	}

	void populateActiveAirports();
	std::vector<std::string> GetActiveAirports() const { return activeAirports; }
	std::vector<std::string> getAllDepartureCallsigns() const;
	bool isDepartureAirport(const std::string& oaci) const;
	bool pilotExists(const std::string& callsign) const;


private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;

	std::vector<std::string> activeAirports;
};