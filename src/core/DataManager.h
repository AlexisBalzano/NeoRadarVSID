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

struct sidData {
	std::string sid;
	int cfl;
};

using namespace PluginSDK;

class DataManager {
public:
	DataManager(vsid::NeoVSID* neoVSID);
	~DataManager() = default;

	void clearData();

	static std::filesystem::path getDllDirectory();
	void DisplayMessageFromDataManager(const std::string& message, const std::string& sender = "");
	void populateActiveAirports();
	int retrieveConfigJson(const std::string& oaci);
	void loadAircraftDataJson();

	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	std::vector<std::string> getAllDepartureCallsigns();
	std::vector<Pilot> getPilots() const { return pilots; }
	Pilot getPilotByCallsign(std::string callsign) const;

	void addPilot(const std::string& callsign);
	bool removePilot(const std::string& callsign);
	void removeAllPilots();

	bool isDepartureAirport(const std::string& oaci) const;
	bool aircraftExists(const std::string& callsign) const;
	bool pilotExists(const std::string& callsign) const;

	std::string generateVRWY(const Flightplan::Flightplan& flightplan);
	sidData generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy);

private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;
	PluginSDK::ControllerData::ControllerDataAPI* controllerDataAPI_ = nullptr;
	Chat::ChatAPI* chatAPI_ = nullptr;
	vsid::NeoVSID* neoVSID_ = nullptr;

	std::filesystem::path configPath_;
	nlohmann::json configJson_;
	nlohmann::json aircraftDataJson_;
	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
};