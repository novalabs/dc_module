/* COPYRIGHT (c) 2016 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include <Core/MW/Middleware.hpp>

#include "ch.h"
#include "hal.h"

#include <Core/HW/GPIO.hpp>
#include <Core/HW/QEI.hpp>
#include <Core/HW/PWM.hpp>
#include <Core/MW/Thread.hpp>
#include <Module.hpp>
#include <QEI_driver/QEI.hpp>
#include <A4957_driver/A4957.hpp>

static Core::HW::QEI_<Core::HW::QEI_4> _encoder;

static Core::HW::PWMChannel_<Core::HW::PWM_1, 0> _pwm_channel_0;
static Core::HW::PWMChannel_<Core::HW::PWM_1, 1> _pwm_channel_1;

using LED_PAD = Core::HW::Pad_<Core::HW::GPIO_F, GPIOF_LED>;
static LED_PAD _led;

using HBRIDGE_RESET_PAD = Core::HW::Pad_<Core::HW::GPIO_B, GPIOB_RESET>;
using HBRIDGE_FAULT_PAD = Core::HW::Pad_<Core::HW::GPIO_A, GPIOB_FAULT>;

static HBRIDGE_RESET_PAD _hbridge_reset;
static HBRIDGE_FAULT_PAD _hbridge_fault;

static sensors::QEI       _qei_device(_encoder);
static sensors::QEI_Delta _qei_delta(_qei_device);

static actuators::A4957 _pwm_device(_pwm_channel_0, _pwm_channel_1, _hbridge_reset, _hbridge_fault);
static actuators::A4957_SignMagnitude _pwm(_pwm_device);

sensors::QEI_Delta& Module::qei = _qei_delta;
actuators::A4957_SignMagnitude& Module::pwm = _pwm;

static THD_WORKING_AREA(wa_info, 1024);
static Core::MW::RTCANTransport rtcantra(RTCAND1);

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

Core::MW::Middleware Core::MW::Middleware::instance(CORE_MODULE_NAME, "BOOT_" CORE_MODULE_NAME);

Module::Module()
{}

bool
Module::initialize()
{
//	CORE_ASSERT(Core::MW::Middleware::instance.is_stopped()); // TODO: capire perche non va...

   static bool initialized = false;

   if (!initialized) {
      halInit();
      qeiInit();

      chSysInit();

      Core::MW::Middleware::instance.initialize(wa_info, sizeof(wa_info), Core::MW::Thread::LOWEST);
      rtcantra.initialize(rtcan_config);
      Core::MW::Middleware::instance.start();


      _encoder.start(&qei_config);
      pwmStart(Core::HW::PWM_1::driver, &pwmcfg);

      initialized = true;
   }

   return initialized;
} // Board::initialize

// Leftover from CoreBoard (where LED_PAD cannot be defined
void
Core::MW::CoreModule::Led::toggle()
{
   _led.toggle();
}

void
Core::MW::CoreModule::Led::write(
   unsigned on
)
{
   _led.write(on);
}
