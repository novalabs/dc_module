/* COPYRIGHT (c) 2016 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#pragma once

#include <Configuration.hpp>
#include <Core/MW/CoreModule.hpp>
#include <Core/MW/CoreSensor.hpp>

#include <functional>

// Forward declarations
namespace sensors {
   class QEI_Delta;
}

namespace actuators {
   class A4957_SignMagnitude;
}

class Module:
   public Core::MW::CoreModule
{
public:
// --- DEVICES ----------------------------------------------------------------
   static sensors::QEI_Delta& qei;
   static actuators::A4957_SignMagnitude& hbridge_pwm;
// ----------------------------------------------------------------------------

   static void
   setPWMCallback(
      std::function<void()>callback
   );                                                          //!< Sets the callback that will be called every PWM cycle

   static void
   resetPWMCallback();

   static bool
   initialize();


   Module();
   virtual ~Module() {}
};
