/* COPYRIGHT (c) 2016-2017 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include <core/mw/Middleware.hpp>
#include <core/mw/transport/RTCANTransport.hpp>

#include <core/snippets/CortexMxFaultHandlers.h>

#include <core/hw/GPIO.hpp>
#include <core/hw/QEI.hpp>
#include <core/hw/PWM.hpp>
#include <core/hw/ADC.hpp>
#include <core/hw/IWDG.hpp>
#include <core/os/Thread.hpp>

#include <Module.hpp>

#include <core/QEI_driver/QEI.hpp>

// LED
static core::hw::Pad_<core::hw::GPIO_F, LED_PIN> _led;

// ENCODER
static core::hw::QEI_<core::hw::QEI_2> _encoder_device_1;
static core::hw::QEI_<core::hw::QEI_4> _encoder_device_2;

// H-BRIDGE
core::hw::PWMMaster_<core::hw::PWM_1> _pwm_master;
core::hw::PWMChannel_<core::hw::PWMMaster_<core::hw::PWM_1>::PWM, 0> _pwm_channel_0;
core::hw::PWMChannel_<core::hw::PWMMaster_<core::hw::PWM_1>::PWM, 1> _pwm_channel_1;
core::hw::Pad_<core::hw::GPIO_B, 14> _hbridge_reset;
core::hw::Pad_<core::hw::GPIO_B, 15> _hbridge_fault;

// ENCODER1
core::hw::Pad_<core::hw::GPIO_A, 0> _e1_a;
core::hw::Pad_<core::hw::GPIO_A, 1> _e1_b;
core::hw::Pad_<core::hw::GPIO_A, 2> _e1_i;
core::hw::Pad_<core::hw::GPIO_A, 3> _e1_analog;

// ENCODER2
static core::hw::Pad_<core::hw::GPIO_A, 6> _e2_a;
core::hw::Pad_<core::hw::GPIO_A, 7>        _e2_b;
core::hw::Pad_<core::hw::GPIO_A, 5>        _e2_i;
core::hw::Pad_<core::hw::GPIO_A, 4>        _e2_analog;

// ADC
static core::hw::ADCConversionGroup_<core::hw::ADC_1, 1, 1> _encoder1_sense_adc;
static core::hw::ADCConversionGroup_<core::hw::ADC_2, 1, 1> _current_sense_adc;
static core::hw::ADCConversionGroup_<core::hw::ADC_3, 1, 1> _temperature_sense_adc;
static core::hw::ADCConversionGroup_<core::hw::ADC_4, 1, 1> _voltage_sense_adc;

// CURRENT SENSE
core::hw::Pad_<core::hw::GPIO_B, 10> _current_sense_fault;

// MODULE DEVICES
core::hw::QEI&       Module::qei1 = _encoder_device_1;
core::hw::QEI&       Module::qei2 = _encoder_device_2;
core::hw::PWMMaster& Module::pwm  = _pwm_master;
core::hw::ADCConversionGroup& Module::current_sense_adc = _current_sense_adc;

core::hw::PWMChannel& Module::hbridge_in1   = _pwm_channel_0;
core::hw::PWMChannel& Module::hbridge_in2   = _pwm_channel_1;
core::hw::Pad&        Module::hbridge_reset = _hbridge_reset;
core::hw::Pad&        Module::hbridge_fault = _hbridge_fault;

core::hw::Pad& Module::current_sense_fault = _current_sense_fault;


core::hw::Pad& Module::Encoder1::a      = _e1_a;
core::hw::Pad& Module::Encoder1::b      = _e1_b;
core::hw::Pad& Module::Encoder1::i      = _e1_i;
core::hw::Pad& Module::Encoder1::analog = _e1_analog;

core::hw::Pad& Module::Encoder2::a      = _e2_a;
core::hw::Pad& Module::Encoder2::b      = _e2_b;
core::hw::Pad& Module::Encoder2::i      = _e2_i;
core::hw::Pad& Module::Encoder2::analog = _e2_analog;

// SYSTEM STUFF
static core::os::Thread::Stack<1024> management_thread_stack;
static core::mw::RTCANTransport      rtcantra(&RTCAND1);

RTCANConfig rtcan_config = {
    1000000, 100, 60
};


Module::Module()
{}

bool
Module::initialize()
{
    FAULT_HANDLERS_ENABLE(true);

    static bool initialized = false;

    if (!initialized) {
        core::mw::CoreModule::initialize();
        qeiInit();

        core::mw::Middleware::instance().initialize(name(), management_thread_stack, management_thread_stack.size(), core::os::Thread::LOWEST);
        rtcantra.initialize(rtcan_config, canID());
        core::mw::Middleware::instance().start();

        initialized = true;
    }

    return initialized;
} // Board::initialize

void
Module::Encoder1::setMode(
    Module::Encoder1::Mode mode
)
{
    switch (mode) {
      case Module::Encoder1::Mode::QEI_ANALOG:
          _e1_a.setMode(core::hw::Pad::Mode::ALTERNATE_1);
          _e1_b.setMode(core::hw::Pad::Mode::ALTERNATE_1);
          _e1_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_analog.setMode(core::hw::Pad::Mode::INPUT_ANALOG);
          break;
      case Module::Encoder1::Mode::GPIO_ANALOG:
          _e1_a.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_b.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_analog.setMode(core::hw::Pad::Mode::INPUT_ANALOG);
          break;
      case Module::Encoder1::Mode::QEI_GPIO:
          _e1_a.setMode(core::hw::Pad::Mode::ALTERNATE_1);
          _e1_b.setMode(core::hw::Pad::Mode::ALTERNATE_1);
          _e1_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_analog.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          break;
      case Module::Encoder1::Mode::GPIO:
      default:
          _e1_a.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_b.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e1_analog.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          break;
    } // switch
} // Module::Encoder1::setMode

void
Module::Encoder2::setMode(
    Module::Encoder2::Mode mode
)
{
    switch (mode) {
      case Module::Encoder2::Mode::QEI_ANALOG:
          _e2_a.setMode(core::hw::Pad::Mode::ALTERNATE_2);
          _e2_b.setMode(core::hw::Pad::Mode::ALTERNATE_2);
          _e2_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_analog.setMode(core::hw::Pad::Mode::INPUT_ANALOG);
          break;
      case Module::Encoder2::Mode::GPIO_ANALOG:
          _e2_a.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_b.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_analog.setMode(core::hw::Pad::Mode::INPUT_ANALOG);
          break;
      case Module::Encoder2::Mode::QEI_GPIO:
          _e2_a.setMode(core::hw::Pad::Mode::ALTERNATE_2);
          _e2_b.setMode(core::hw::Pad::Mode::ALTERNATE_2);
          _e2_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_analog.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          break;
      case Module::Encoder2::Mode::SPI:
          _e2_a.setMode(core::hw::Pad::Mode::ALTERNATE_5);
          _e2_b.setMode(core::hw::Pad::Mode::ALTERNATE_5);
          _e2_i.setMode(core::hw::Pad::Mode::ALTERNATE_5);
          _e2_analog.setMode(core::hw::Pad::Mode::ALTERNATE_5);
          break;
      case Module::Encoder2::Mode::GPIO:
      default:
          _e2_a.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_b.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_i.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          _e2_analog.setMode(core::hw::Pad::Mode::INPUT_PULLUP);
          break;
    } // switch
} // Module::Encoder2::setMode

// ----------------------------------------------------------------------------
// CoreModule STM32FlashConfigurationStorage
// ----------------------------------------------------------------------------
#include <core/snippets/CoreModuleSTM32FlashConfigurationStorage.hpp>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// CoreModule HW specific implementation
// ----------------------------------------------------------------------------
#include <core/snippets/CoreModuleHWSpecificImplementation.hpp>
// ----------------------------------------------------------------------------
