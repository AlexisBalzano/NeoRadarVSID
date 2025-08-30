#pragma once
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <mutex>
#include <unordered_set>

#include "SDK.h"

struct Pilot {
	std::string callsign;
	std::string rwy;
	std::string sid;
	std::string oaci;
	int cfl;

	bool empty() const {
		return callsign.empty();
	}
};

struct sidData {
	std::string rwy;
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
	bool isCorrectJsonVersion(const std::string& config_version, const std::string& fileName);
	void loadAircraftDataJson();
	void parseRules(const std::string& oaci);
	void parseAreas(const std::string& oaci);

	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	std::vector<std::string> getAllDepartureCallsigns();
	std::vector<Pilot> getPilots() const { return pilots; }
	Pilot getPilotByCallsign(std::string callsign);
	std::vector<ruleData> getRules() const { return rules; }
	std::vector<areaData> getAreas() const { return areas; }
	int getTransAltitude(const std::string& oaci);

	void switchRuleState(const std::string& oaci, const std::string& ruleName);
	void switchAreaState(const std::string& oaci, const std::string& areaName);
	void addPilot(const std::string& callsign);
	bool removePilot(const std::string& callsign);
	void removeAllPilots();

	bool isDepartureAirport(const std::string& oaci);
	bool aircraftExists(const std::string& callsign) const;
	bool pilotExists(const std::string& callsign);
	bool isInArea(const double& latitude, const double& longitude, const std::string& oaci, const std::string& areaName);
	bool isMatchingRules(const nlohmann::ordered_json waypointSidData, const std::vector<std::string> activeRules, const std::string& letter, const std::string& variant);
	bool isMatchingAreas(const nlohmann::ordered_json waypointSidData, const std::vector<std::string> activeAreas, const std::string& letter, const std::string& variant, const Flightplan::Flightplan fp);
	bool isMatchingEngineRestrictions(const nlohmann::ordered_json sidData, const std::string& aircraftType);
	bool isRNAV(const std::string& aircraftType);

	int fetchCFL(const Flightplan::Flightplan& flightplan, const std::vector<std::string> activeRules, const std::vector<std::string> activeAreas, const std::string& vsid, bool singleRwy);
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
	nlohmann::ordered_json configJson_;
	nlohmann::json aircraftDataJson_;
	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
	std::vector<ruleData> rules;
	std::vector<areaData> areas;

	std::unordered_set<std::string> configsError;

	std::mutex dataMutex_;

};