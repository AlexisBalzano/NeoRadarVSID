#pragma once
#include <memory>
#include <thread>
#include <vector>

#include "NeoRadarSDK/SDK.h"
#include "core/NeoVSIDCommandProvider.h"
#include "core/DataManager.h"
#include "utils/Color.h"

constexpr const char* NEOVSID_VERSION = "v1.3.6";

using namespace PluginSDK;

class DataManager;

namespace vsid {
    struct tagUpdateParam
    {
        std::string callsign;
        Pilot pilot;
        PluginSDK::ControllerData::ControllerDataAPI* controllerDataAPI_;
        Tag::TagInterface* tagInterface_;
        std::string tagId_;
    };

    class NeoVSIDCommandProvider;

    class NeoVSID : public BasePlugin
    {
    public:
        NeoVSID();
        ~NeoVSID();

		// Plugin lifecycle methods
        void Initialize(const PluginMetadata& metadata, CoreAPI* coreAPI, ClientInformation info) override;
        std::pair<bool, std::string> newVersionAvailable();
        void Shutdown() override;
        void Reset();
        PluginMetadata GetMetadata() const override;

        // Radar commands
        void DisplayMessage(const std::string& message, const std::string& sender = "");
		/* void SetSid(const std::string& callsign, const std::string& sid);
           void SetCFL(const std::string& callsign, const std::string& cfl);
		   void SetRunway(const std::string& callsign, const std::string& rwy);*/
        
        // Scope events
        void OnControllerDataUpdated(const ControllerData::ControllerDataUpdatedEvent* event) override;
        void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) override;
        void OnAircraftTemporaryAltitudeChanged(const ControllerData::AircraftTemporaryAltitudeChangedEvent* event) override;
        void OnPositionUpdate(const Aircraft::PositionUpdateEvent* event) override;
        void OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event) override;
        void OnFlightplanRemoved(const Flightplan::FlightplanRemovedEvent* event) override;
        void OnTimer(int Counter);

        // Command handling
        void TagProcessing(const std::string& callsign, const std::string& actionId, const std::string& userInput = "");
        bool getAutoModeState() const { return autoModeState; }
        void switchAutoModeState() { autoModeState = !autoModeState; }

		// API Accessors
        PluginSDK::Logger::LoggerAPI* GetLogger() const { return logger_; }
        Aircraft::AircraftAPI* GetAircraftAPI() const { return aircraftAPI_; }
        Airport::AirportAPI* GetAirportAPI() const { return airportAPI_; }
        Chat::ChatAPI* GetChatAPI() const { return chatAPI_; }
        Flightplan::FlightplanAPI* GetFlightplanAPI() const { return flightplanAPI_; }
        Fsd::FsdAPI* GetFsdAPI() const { return fsdAPI_; }
        PluginSDK::ControllerData::ControllerDataAPI* GetControllerDataAPI() const { return controllerDataAPI_; }
		Tag::TagInterface* GetTagInterface() const { return tagInterface_; }
        DataManager* GetDataManager() const { return dataManager_.get(); }

    private:
        void runScopeUpdate();
        void run();
        std::pair<std::string, size_t> getRequestAndIndex(const std::string& callsign);

    public:
        // Command IDs
        std::string helpCommandId_;
        std::string versionCommandId_;
        std::string autoModeCommandId_;
        std::string airportsCommandId_;
        std::string pilotsCommandId_;
		std::string areasCommandId_;
		std::string rulesCommandId_;
        std::string resetCommandId_;
        std::string removeCommandId_;
        std::string positionCommandId_;
        std::string areaCommandId_;
        std::string ruleCommandId_;


    private:
        // Plugin state
        std::vector<std::string> callsignsScope;
		std::mutex callsignsMutex;
        bool initialized_ = false;
		bool autoModeState = true; // auto update every 5 seconds (should be true when standard ops)
        std::thread m_worker;
        bool m_stop;
        std::vector<std::string> requestingClearance;
        std::vector<std::string> requestingPush;
        std::vector<std::string> requestingTaxi;

        // APIs
        PluginMetadata metadata_;
        ClientInformation clientInfo_;
        Aircraft::AircraftAPI* aircraftAPI_ = nullptr;
        Airport::AirportAPI* airportAPI_ = nullptr;
        Chat::ChatAPI* chatAPI_ = nullptr;
        Flightplan::FlightplanAPI* flightplanAPI_ = nullptr;
        Fsd::FsdAPI* fsdAPI_ = nullptr;
        PluginSDK::Logger::LoggerAPI* logger_ = nullptr;
        PluginSDK::ControllerData::ControllerDataAPI* controllerDataAPI_ = nullptr;
        Tag::TagInterface* tagInterface_ = nullptr;
        std::unique_ptr<DataManager> dataManager_;
        std::shared_ptr<NeoVSIDCommandProvider> CommandProvider_;

        // Tag Items
        void RegisterTagItems();
        void RegisterTagActions();
        void RegisterCommand();
        void unegisterCommand();
        void OnTagAction(const Tag::TagActionEvent* event) override;
        void OnTagDropdownAction(const Tag::DropdownActionEvent* event) override;
        void UpdateTagItems();
        void UpdateTagItems(std::string Callsign);
        void updateCFL(tagUpdateParam param);
        Color colorizeCfl(const int& cfl, const int& vsidCfl);
        Color colorizeRwy(const std::string& rwy, const std::string& vsidRwy, const bool& isDepRwy);
        Color colorizeSid(const std::string& sid, const std::string& vsidSid);
        Color colorizeAlert();
        Color colorizeRequest();
        void updateRWY(tagUpdateParam param);
        void updateSID(tagUpdateParam param);
        void updateAlert(const std::string& callsign);
		void updateRequest(const std::string& callsign, const std::string& request);
        void updateAllRequests();

	    // TAG Items IDs
		std::string cflId_;
        std::string rwyId_;
        std::string sidId_;
		std::string alertsId_;
        std::string requestId_;
		std::string requestMenuId_;

        // TAG Action IDs
        std::string confirmRwyId_;
        std::string confirmSidId_;
        std::string confirmCFLId_;
    };
} // namespace vsid