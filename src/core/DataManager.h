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

	bool empty() const {
		return callsign.empty();
	}
};


using namespace PluginSDK;
class DataManager {
public:
	DataManager(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI);
	~DataManager() = default;

	static std::filesystem::path getDllDirectory();
	
	void clearData();

	void populateActiveAirports();
	int fetchCFLfromJson(const Flightplan::Flightplan& flightplan);
	int retrieveConfigJson(const std::string& oaci);
	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	Pilot getPilotByCallsign(std::string callsign) const;
	std::vector<std::string> getAllDepartureCallsigns();
	bool isDepartureAirport(const std::string& oaci) const;
	bool aircraftExists(const std::string& callsign) const;
	bool pilotExists(const std::string& callsign) const;
	void addPilot(const std::string& callsign);


private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;

	std::filesystem::path configPath_;

	nlohmann::json configJson_;

	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
};