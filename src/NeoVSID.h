// NeoVSID.h
#pragma once
#include <memory>
#include <thread>
#include <vector>
#include "SDK.h"
#include "core/NeoVSIDCommandProvider.h"
#include "core/DataManager.h"


using namespace PluginSDK;

namespace vsid {

    struct tagUpdateParam
    {
        std::string callsign;
        Pilot pilot;
        PluginSDK::ControllerData::ControllerDataAPI* controllerDataAPI_;
        Tag::TagInterface* tagInterface_;
        std::string tagId_;
        bool confirmValue; // if true, vsid CFL/RWY/SID will be printed in FP
    };

    class NeoVSIDCommandProvider;

    class NeoVSID : public BasePlugin
    {
    public:
        NeoVSID();
        ~NeoVSID();

        void Initialize(const PluginMetadata& metadata, CoreAPI* coreAPI, ClientInformation info) override;
        void Shutdown() override;
        PluginMetadata GetMetadata() const override;

        // Radar commands
        void DisplayMessage(const std::string& message, const std::string& sender = "");
        void SetGroundState(const std::string& callsign, const ControllerData::GroundStatus groundstate);
		/* void SetSid(const std::string& callsign, const std::string& sid);
           void SetCFL(const std::string& callsign, const std::string& cfl);
		   void SetRunway(const std::string& callsign, const std::string& rwy);
        */
        
        // Scope events
        void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) override;
        void OnAircraftTemporaryAltitudeChanged(const ControllerData::AircraftTemporaryAltitudeChangedEvent* event) override;
        virtual void OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event) override;
        void OnTimer(int Counter);
        /*    bool OnCompileCommand(const char *sCommandLine) override;*/

        // Command handling
        void TagProcessing(const std::string& callsign, const std::string& actionId, const std::string& userInput = "");
        bool getAutoModeState() const { return autoModeState; }
        void switchAutoModeState() { autoModeState = !autoModeState; }

    private:
        bool initialized_ = false;
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


        void runScopeUpdate();

        // Tag Items
        void RegisterTagItems();
        void RegisterTagActions();
        void RegisterCommand();
        void OnTagAction(const Tag::TagActionEvent* event) override;
        void OnTagDropdownAction(const Tag::DropdownActionEvent* event) override;
        void UpdateTagItems();
        void UpdateTagItems(std::string Callsign);
        void updateCFL(tagUpdateParam param);
        void updateRWY(tagUpdateParam param);
        void updateSID(tagUpdateParam param);

		bool autoModeState = false; // Auto mode automatically assigns RWY SID & CFL using API without user confirmation


	    // TAG Items IDs
		std::string cflId_;
        std::string rwyId_;
        std::string sidId_;

        // TAG Action IDs
        std::string confirmRwyId_;
        std::string confirmSidId_;
        std::string confirmCFLId_;

		// Command IDs
		std::string commandId_;

        // Concerned callsigns
        std::vector<std::string> callsignsScope;


        std::thread m_worker;
        bool m_stop;
        void run();

        std::shared_ptr<NeoVSIDCommandProvider> CommandProvider_;
    };

} // namespace vsid