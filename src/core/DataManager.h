#pragma once
#include <vector>

#include "SDK.h"

struct Pilot {
	std::string callsign;
	std::string rwy;
	std::string sid;
	int cfl;
};

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
	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	Pilot getPilotByCallsign(std::string callsign) const;
	std::vector<std::string> getAllDepartureCallsigns();
	bool isDepartureAirport(const std::string& oaci) const;
	bool pilotExists(const std::string& callsign) const;


private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;

	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
};