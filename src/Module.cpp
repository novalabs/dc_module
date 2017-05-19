/* COPYRIGHT (c) 2016-2017 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include <core/mw/Middleware.hpp>
#include <core/mw/transport/RTCANTransport.hpp>

#include "ch.h"
#include "hal.h"
#include <core/snippets/CortexMxFaultHandlers.h>

#include <core/hw/GPIO.hpp>
#include <core/hw/QEI.hpp>
#include <core/hw/PWM.hpp>
#include <core/hw/IWDG.hpp>
#include <core/os/Thread.hpp>

#include <Module.hpp>

#include <core/QEI_driver/QEI.hpp>
#include <core/A4957_driver/A4957.hpp>

// LED
using LED_PAD = core::hw::Pad_<core::hw::GPIO_F, GPIOF_LED>;
static LED_PAD _led;

// ENCODER
using ENCODER_DEVICE = core::hw::QEI_<core::hw::QEI_2>;
static ENCODER_DEVICE              _encoder_device;
static core::QEI_driver::QEI       _qei_device(_encoder_device);
static core::QEI_driver::QEI_Delta _qei_delta("m_encoder", _qei_device);

// H-BRIDGE
using PWM_MASTER        = core::hw::PWMMaster_<core::hw::PWM_1>;
using PWM_CHANNEL_0     = core::hw::PWMChannel_<PWM_MASTER::PWM, 0>;
using PWM_CHANNEL_1     = core::hw::PWMChannel_<PWM_MASTER::PWM, 1>;
using HBRIDGE_RESET_PAD = core::hw::Pad_<core::hw::GPIO_B, GPIOB_RESET>;
using HBRIDGE_FAULT_PAD = core::hw::Pad_<core::hw::GPIO_B, GPIOB_FAULT>;
static PWM_MASTER                _pwm_master;
static PWM_CHANNEL_0             _pwm_channel_0;
static PWM_CHANNEL_0             _pwm_channel_1;
static HBRIDGE_RESET_PAD         _hbridge_reset;
static HBRIDGE_FAULT_PAD         _hbridge_fault;
static core::A4957_driver::A4957 _pwm_device(_pwm_master, _pwm_channel_0, _pwm_channel_1, _hbridge_reset, _hbridge_fault);
static core::A4957_driver::A4957_SignMagnitude _h_bridge("m_motor", _pwm_device);

// MODULE DEVICES
core::hw::QEI& Module::qei = _encoder_device;
core::QEI_driver::QEI_Delta& Module::encoder = _qei_delta;
core::hw::PWMMaster&         Module::pwm     = _pwm_master;
core::A4957_driver::A4957_SignMagnitude& Module::h_bridge = _h_bridge;


// SYSTEM STUFF
static core::os::Thread::Stack<1024> management_thread_stack;
static core::mw::RTCANTransport      rtcantra(&RTCAND1);

core::mw::Middleware
core::mw::Middleware::instance(
    ModuleConfiguration::MODULE_NAME
);


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
        halInit();
        qeiInit();

        chSysInit();

        core::mw::Middleware::instance.initialize(name(), management_thread_stack, management_thread_stack.size(), core::os::Thread::LOWEST);
        rtcantra.initialize(rtcan_config, canID());
        core::mw::Middleware::instance.start();

        initialized = true;
    }

    return initialized;
} // Board::initialize

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
