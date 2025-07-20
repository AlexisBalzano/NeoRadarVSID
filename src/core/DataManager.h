#pragma once
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

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
	DataManager(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI);
	~DataManager() = default;

	
	static DataManager* instance(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI) {
		static DataManager instance(aircraftAPI, flightplanAPI, airportAPI);
		return &instance;
	}
	static std::filesystem::path getDllDirectory();

	void populateActiveAirports();
	int fetchCFLfromJson(const Flightplan::Flightplan& flightplan);
	int retrieveConfigJson(const std::string& oaci);
	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	Pilot getPilotByCallsign(std::string callsign) const;
	std::vector<std::string> getAllDepartureCallsigns();
	bool isDepartureAirport(const std::string& oaci) const;
	bool pilotExists(const std::string& callsign) const;


private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;

	std::filesystem::path configPath_;

	nlohmann::json configJson_;

	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
};