#include <opm/parser/eclipse/EclipseState/Schedule/WellProductionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>

#include <string>
#include <vector>

namespace Opm {
    WellProductionProperties::
    WellProductionProperties()
    {
        init();

        predictionMode = true;
    }

    WellProductionProperties::
    WellProductionProperties(DeckRecordConstPtr record)
    {
        init();

        WaterRate = record->getItem("WRAT")->getSIDouble(0);
        OilRate   = record->getItem("ORAT")->getSIDouble(0);
        GasRate   = record->getItem("GRAT")->getSIDouble(0);
    }

    WellProductionProperties
    WellProductionProperties::history(DeckRecordConstPtr record)
    {
        WellProductionProperties p(record);

        p.predictionMode = false;

        // Modes supported in WCONHIST just from {O,W,G}RAT values
        //
        // Note: The default value of observed {O,W,G}RAT is zero
        // (numerically) whence the following control modes are
        // unconditionally supported.
        const std::vector<std::string> controlModes{
            "ORAT", "WRAT", "GRAT", "LRAT", "RESV"
        };

        for (std::vector<std::string>::const_iterator
                 mode = controlModes.begin(), end = controlModes.end();
             mode != end; ++mode)
        {
            const WellProducer::ControlModeEnum cmode =
                WellProducer::ControlModeFromString(*mode);

            p.addProductionControl(cmode);
        }

        // BHP control must be explictly provided.
        if (!record->getItem("BHP")->defaultApplied(0)) {
            p.addProductionControl(WellProducer::BHP);
        }

        {
            const auto cmodeItem = record->getItem("CMODE");
            if (!cmodeItem->defaultApplied(0)) {
                const WellProducer::ControlModeEnum cmode = WellProducer::ControlModeFromString( cmodeItem->getString(0) );

                if (p.hasProductionControl( cmode ))
                    p.controlMode = cmode;
                else
                    throw std::invalid_argument("Setting CMODE to unspecified control");
            }
        }

        return p;
    }

    WellProductionProperties
    WellProductionProperties::prediction(DeckRecordConstPtr record)
    {
        WellProductionProperties p(record);

        p.predictionMode = true;

        p.LiquidRate = record->getItem("LRAT")->getSIDouble(0);
        p.ResVRate   = record->getItem("RESV")->getSIDouble(0);
        p.BHPLimit   = record->getItem("BHP" )->getSIDouble(0);
        p.THPLimit   = record->getItem("THP" )->getSIDouble(0);

        const std::vector<std::string> controlModes{
            "ORAT", "WRAT", "GRAT", "LRAT",
            "RESV", "BHP" , "THP"
        };

        for (std::vector<std::string>::const_iterator
                 mode = controlModes.begin(), end = controlModes.end();
             mode != end; ++mode)
        {
            if (!record->getItem(*mode)->defaultApplied(0)) {
                const WellProducer::ControlModeEnum cmode =
                    WellProducer::ControlModeFromString(*mode);

                p.addProductionControl(cmode);
            }
        }

        {
            const auto cmodeItem = record->getItem("CMODE");
            if (!cmodeItem->defaultApplied(0)) {
                const WellProducer::ControlModeEnum cmode = WellProducer::ControlModeFromString( cmodeItem->getString(0) );

                if (p.hasProductionControl( cmode ))
                    p.controlMode = cmode;
                else
                    throw std::invalid_argument("Setting CMODE to unspecified control");
            }
        }

        return p;
    }

    void
    WellProductionProperties::init()
    {
        // public: properties (in order of declaration)
        OilRate     = 0.0;
        GasRate     = 0.0;
        WaterRate   = 0.0;
        LiquidRate  = 0.0;
        ResVRate    = 0.0;
        BHPLimit    = 0.0;
        THPLimit    = 0.0;
        controlMode = WellProducer::CMODE_UNDEFINED;

        // private: property
        productionControls = 0;
    }
} // namespace Opm
