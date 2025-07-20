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

    class NeoVSIDCommandProvider;

    class NeoVSID : public BasePlugin
    {
    public:
        NeoVSID();
        ~NeoVSID();

        void Initialize(const PluginMetadata& metadata, CoreAPI* coreAPI, ClientInformation info) override;
        void Shutdown() override;
        PluginMetadata GetMetadata() const override;

        void DisplayMessage(const std::string& message, const std::string& sender = "");
        void SetGroundState(const std::string& callsign, const ControllerData::GroundStatus groundstate);

        // Scope events
        void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) override;
        void OnAircraftTemporaryAltitudeChanged(const ControllerData::AircraftTemporaryAltitudeChangedEvent* event) override;
        virtual void OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event) override;
        void OnTimer(int Counter);
        void OnTagAction(const Tag::TagActionEvent* event) override;
        void OnTagDropdownAction(const Tag::DropdownActionEvent* event) override;
        void UpdateTagItems();
        void UpdateTagItems(std::string Callsign);
        /*    bool OnCompileCommand(const char *sCommandLine) override;*/

        // Command handling
        void TagProcessing(const std::string& callsign, const std::string& actionId, const std::string& userInput = "");

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
		DataManager* dataManager_ = nullptr;

        std::optional<Aircraft::Aircraft> GetAircraftByCallsign(const std::string& callsign);


        void runScopeUpdate();

        void RegisterTagItems();
        void RegisterTagActions();
        void RegisterCommand();


        std::thread m_worker;
        bool m_stop;
        void run();

        std::shared_ptr<NeoVSIDCommandProvider> CommandProvider_;

        // Concerned callsigns
        std::vector<std::string> callsignsScope;

	    // TAG Items IDs
		std::string cflId_;
        std::string rwyId_;
        std::string sidId_;

        // TAG Action IDs
        std::string confirmRwyId_;
        std::string confirmSidId_;
        std::string confirmCFLId_;

    };

} // namespace vsid