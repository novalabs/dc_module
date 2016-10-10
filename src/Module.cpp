/* COPYRIGHT (c) 2016 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include <core/mw/Middleware.hpp>
#include <core/mw/transport/RTCANTransport.hpp>

#include "ch.h"
#include "hal.h"

#include <core/hw/GPIO.hpp>
#include <core/hw/QEI.hpp>
#include <core/hw/PWM.hpp>
#include <core/os/Thread.hpp>
#include <Module.hpp>
#include <core/QEI_driver/QEI.hpp>
#include <core/A4957_driver/A4957.hpp>

static core::hw::QEI_<core::hw::QEI_2> _encoder;

static core::hw::PWMMaster_<core::hw::PWM_1> _pwm_1;

static core::hw::PWMChannel_<core::hw::PWM_1, 0> _pwm_channel_0;
static core::hw::PWMChannel_<core::hw::PWM_1, 1> _pwm_channel_1;

using LED_PAD = core::hw::Pad_<core::hw::GPIO_F, GPIOF_LED>;
static LED_PAD _led;

using HBRIDGE_RESET_PAD = core::hw::Pad_<core::hw::GPIO_B, GPIOB_RESET>;
using HBRIDGE_FAULT_PAD = core::hw::Pad_<core::hw::GPIO_B, GPIOB_FAULT>;

static HBRIDGE_RESET_PAD _hbridge_reset;
static HBRIDGE_FAULT_PAD _hbridge_fault;

static core::QEI_driver::QEI       _qei_device(_encoder);
static core::QEI_driver::QEI_Delta _qei_delta("m_encoder", _qei_device);

static core::A4957_driver::A4957 _pwm_device(_pwm_1, _pwm_channel_0, _pwm_channel_1, _hbridge_reset, _hbridge_fault);
static core::A4957_driver::A4957_SignMagnitude _pwm("m_pwm", _pwm_device);

core::QEI_driver::QEI_Delta& Module::qei = _qei_delta;
core::A4957_driver::A4957_SignMagnitude& Module::hbridge_pwm = _pwm;

static core::os::Thread::Stack<1024> management_thread_stack;
static core::mw::RTCANTransport      rtcantra(&RTCAND1);

RTCANConfig rtcan_config = {
   1000000, 100, 60
};

QEIConfig qei_config = {
   QEI_MODE_QUADRATURE, QEI_BOTH_EDGES, QEI_DIRINV_FALSE,
};

/*
 * PWM configuration.
 */
static PWMConfig pwmcfg = {
   72000000,/* 72MHz PWM clock.   */
   4096,    /* 12-bit PWM, 18KHz frequency. */
   nullptr, {
      {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH,NULL},
      {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH,NULL},
      {PWM_OUTPUT_DISABLED,NULL},
      {PWM_OUTPUT_DISABLED,NULL}
   }, 0, 72
};

#ifndef CORE_MODULE_NAME
#define CORE_MODULE_NAME "UDC"
#endif

core::mw::Middleware core::mw::Middleware::instance(CORE_MODULE_NAME, "BOOT_" CORE_MODULE_NAME);

Module::Module()
{}

bool
Module::initialize()
{
//	CORE_ASSERT(core::mw::Middleware::instance.is_stopped()); // TODO: capire perche non va...

   static bool initialized = false;

   if (!initialized) {
      halInit();
      qeiInit();

      chSysInit();

      core::mw::Middleware::instance.initialize(management_thread_stack, management_thread_stack.size(), core::os::Thread::LOWEST);
      rtcantra.initialize(rtcan_config);
      core::mw::Middleware::instance.start();


      _encoder.start(&qei_config);
      pwmStart(core::hw::PWM_1::driver, &pwmcfg);

      initialized = true;
   }

   return initialized;
} // Board::initialize

void
Module::setPWMCallback(
   std::function<void()>callback
)
{
   _pwm_1.setCallback(callback);
}

void
Module::resetPWMCallback()
{
   _pwm_1.resetCallback();
}

// Leftover from CoreBoard (where LED_PAD cannot be defined
void
core::mw::CoreModule::Led::toggle()
{
   _led.toggle();
}

void
core::mw::CoreModule::Led::write(
   unsigned on
)
{
   _led.write(on);
}
