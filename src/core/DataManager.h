#pragma once
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <mutex>
#include <unordered_set>

#include "./utils/Color.h"

namespace vsid
{
	// Default settings values
	constexpr int DEFAULT_UPDATE_INTERVAL = 5; // seconds
	constexpr int ALERT_MAX_ALTITUDE = 5000; // Max altitude to show ground alerts
	constexpr double MAX_DISTANCE = 4.; //Max distance from origin airport for auto assigning SID/CFL/RWY
}

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

	std::filesystem::path getDllDirectory();
	void DisplayMessageFromDataManager(const std::string& message, const std::string& sender = "");
	void populateActiveAirports();
	int retrieveAirportConfigJson(const std::string& oaci);
	bool retrieveCorrectAirportConfigJson(const std::string& oaci);
	void loadAircraftDataJson();
	void loadConfigJson();
	void loadCustomAssignJson();
	void parseRules(const std::string& oaci);
	void parseAreas(const std::string& oaci);
	bool parseSettings();
	void useDefaultColors();
	void setUpdateInterval(const int& interval) { updateInterval_ = interval; }
	void setAlertMaxAltitude(const int& alt) { alertMaxAltitude_ = alt; }
	void setMaxAircraftDistance(const double& dist) { maxAircraftDistance_ = dist; }
	bool saveDownloadedAirportConfig(const nlohmann::ordered_json& json, std::string icao);

	std::vector<std::string> getActiveAirports() const { return activeAirports; }
	std::vector<std::string> getAllDepartureCallsigns();
	std::vector<Pilot> getPilots() const { return pilots; }
	Pilot getPilotByCallsign(std::string callsign);
	std::vector<ruleData> getRules() const { return rules; }
	std::vector<areaData> getAreas() const { return areas; }
	int getTransAltitude(const std::string& oaci);
	vsid::Color getColor(const vsid::ColorName& colorName);
	int getUpdateInterval() const { return updateInterval_; }
	int getAlertMaxAltitude() const { return alertMaxAltitude_; }
	double getMaxAircraftDistance() const { return maxAircraftDistance_; }
	std::string getConfigUrl() const { return configUrl_; }
#ifdef DEV
	std::string getPushInfo(const std::string& callsign);
#endif // DEV

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
	bool customAssignExists() const;

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
	Package::PackageAPI* packageAPI_ = nullptr;

	std::filesystem::path configPath_;
	std::filesystem::path datasetPath_;
	nlohmann::ordered_json airportConfigJson_;
	nlohmann::json aircraftDataJson_;
	nlohmann::json customAssignJson_;
	nlohmann::json configJson_;
	std::vector<std::string> activeAirports;
	std::vector<Pilot> pilots;
	std::vector<ruleData> rules;
	std::vector<areaData> areas;
	std::array<vsid::Color, 11> colors_;
	int updateInterval_;
	int alertMaxAltitude_;
	double maxAircraftDistance_;
	std::string configUrl_;

	std::unordered_set<std::string> configsError_;
	std::unordered_set<std::string> configsDownloaded_;

	std::mutex dataMutex_;

	// Default Colors
	vsid::Color green_ = std::array<unsigned int, 3>{ 127, 252, 73 };
	vsid::Color white_ = std::array<unsigned int, 3>({ 255, 255, 255 });
	vsid::Color red_ = std::array<unsigned int, 3>({ 240, 0, 0 });
	vsid::Color orange_ = std::array<unsigned int, 3>({ 255, 153, 51 });
	vsid::Color alertBackground_ = std::array<unsigned int, 3>({ 234, 171, 24 });
	vsid::Color strongAlertBackground_ = std::array<unsigned int, 3>({ 88, 8, 9 });
};