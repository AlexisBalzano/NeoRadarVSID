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

struct ruleData {
	std::string oaci;
	std::string name;
	bool active = false; 
};

struct areaData {
	std::string oaci;
	std::string name;
	std::vector <std::pair<double, double>> coordinates;
	bool active = false; 
};

using namespace PluginSDK;

class DataManager {
public:
	DataManager(vsid::NeoVSID* neoVSID);
	~DataManager() = default;

	void clearData();
	void clearJson();

	static std::filesystem::path getDllDirectory();
	void DisplayMessageFromDataManager(const std::string& message, const std::string& sender = "");
	void populateActiveAirports();
	int retrieveConfigJson(const std::string& oaci);
	bool retrieveCorrectConfigJson(const std::string& oaci);
	void loadAircraftDataJson();
	void parseRules(const std::string& oaci);
	void parseAreas(const std::string& oaci);

	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	std::vector<std::string> getAllDepartureCallsigns();
	std::vector<Pilot> getPilots() const { return pilots; }
	Pilot getPilotByCallsign(std::string callsign) const;
	std::vector<ruleData> getRules() const { return rules; }
	std::vector<areaData> getAreas() const { return areas; }

	void switchRuleState(const std::string& oaci, const std::string& ruleName);
	void switchAreaState(const std::string& oaci, const std::string& areaName);
	void addPilot(const std::string& callsign);
	bool removePilot(const std::string& callsign);
	void removeAllPilots();

	bool isDepartureAirport(const std::string& oaci) const;
	bool aircraftExists(const std::string& callsign) const;
	bool pilotExists(const std::string& callsign) const;
	bool isInArea(const double& latitude, const double& longitude, const std::string& oaci, const std::string& areaName);

	std::string generateVRWY(const Flightplan::Flightplan& flightplan);
	sidData generateVSID(const Flightplan::Flightplan& flightplan, const std::string& depRwy);

private:
	Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
	Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
	Airport::AirportAPI* airportAPI_ = nullptr;
	PluginSDK::ControllerData::ControllerDataAPI* controllerDataAPI_ = nullptr;
	Chat::ChatAPI* chatAPI_ = nullptr;
	vsid::NeoVSID* neoVSID_ = nullptr;
	PluginSDK::Logger::LoggerAPI* loggerAPI_ = nullptr;

	std::filesystem::path configPath_;
	nlohmann::json configJson_;
	nlohmann::json aircraftDataJson_;
	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
	std::vector<ruleData> rules;
	std::vector<areaData> areas;
};