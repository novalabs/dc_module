/* COPYRIGHT (c) 2016-2018 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#pragma once

#include <ModuleConfiguration.hpp>
#include <core/mw/CoreModule.hpp>

// Forward declarations
namespace core {
namespace hw {
class QEI;
class Pad;
class PWMMaster;
class PWMChannel;
class ADCConversionGroup;
}
}

class Module:
    public core::mw::CoreModule
{
public:
// --- DEVICES ----------------------------------------------------------------
    static core::hw::QEI&       qei1;
    static core::hw::QEI&       qei2;
    static core::hw::PWMMaster& pwm;

    static core::hw::ADCConversionGroup& encoder1_sense_adc;
    static core::hw::ADCConversionGroup& current_sense_adc;
    static core::hw::ADCConversionGroup& temperature_sense_adc;
    static core::hw::ADCConversionGroup& voltage_sense_adc;

    static core::hw::Pad& current_sense_fault;

    // H BRIDGE
    static core::hw::PWMChannel& hbridge_in1;
    static core::hw::PWMChannel& hbridge_in2;
    static core::hw::Pad&        hbridge_reset;
    static core::hw::Pad&        hbridge_fault;

    class Encoder1
    {
    public:
        enum class Mode {
            QEI_ANALOG,
            QEI_GPIO,
            GPIO_ANALOG,
            GPIO
        };

        static void
        setMode(
            Mode mode
        );


        static core::hw::Pad& a;
        static core::hw::Pad& b;
        static core::hw::Pad& i;
        static core::hw::Pad& analog;
    };

    static Encoder1& encoder1;

    class Encoder2
    {
    public:
        enum class Mode {
            QEI_ANALOG,
            QEI_GPIO,
            GPIO_ANALOG,
            GPIO,
            SPI
        };

        static void
        setMode(
            Mode mode
        );


        static core::hw::Pad& a;
        static core::hw::Pad& b;
        static core::hw::Pad& i;
        static core::hw::Pad& analog;
    };

    enum class Encoder2Mode {
        QEI_ANALOG,
        QEI_GPIO,
        GPIO_ANALOG,
        GPIO,
        SPI
    };

    static Encoder2& encoder2;
// ----------------------------------------------------------------------------

    static bool
    initialize();


    Module();
    virtual ~Module() {}
};
