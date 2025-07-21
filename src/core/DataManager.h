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
	DataManager(Aircraft::AircraftAPI* aircraftAPI, Flightplan::FlightplanAPI* flightplanAPI, Airport::AirportAPI* airportAPI, Chat::ChatAPI* chatAPI);
	~DataManager() = default;

	static std::filesystem::path getDllDirectory();
	void clearData();
	

	void DisplayMessageFromDataManager(const std::string& message, const std::string& sender = "");
	void populateActiveAirports();
	int retrieveConfigJson(const std::string& oaci);
	
	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	std::vector<std::string> getAllDepartureCallsigns();

	bool isDepartureAirport(const std::string& oaci) const;

	bool aircraftExists(const std::string& callsign) const;
	bool pilotExists(const std::string& callsign) const;
	Pilot getPilotByCallsign(std::string callsign) const;
	void addPilot(const std::string& callsign);
	void removePilot(const std::string& callsign);

	int fetchCFLfromJson(const Flightplan::Flightplan& flightplan); // Used while generateVSID is not implemented because it should also return the CFL
	std::string generateVRWY(const Flightplan::Flightplan& flightplan);
	std::string generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy);

private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;
	Chat::ChatAPI* chatAPI_ = nullptr;

	std::filesystem::path configPath_;
	nlohmann::json configJson_;
	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
};