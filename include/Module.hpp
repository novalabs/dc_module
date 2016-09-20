/* COPYRIGHT (c) 2016 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#pragma once

#include <ModuleConfiguration.hpp>
#include <core/mw/CoreModule.hpp>

#include <functional>

// Forward declarations
namespace core {
namespace QEI_driver {
class QEI_Delta;
}
}

namespace core {
namespace A4957_driver {
class A4957_SignMagnitude;
}
}

class Module:
   public core::mw::CoreModule
{
public:
// --- DEVICES ----------------------------------------------------------------
   static core::QEI_driver::QEI_Delta& qei;
   static core::A4957_driver::A4957_SignMagnitude& hbridge_pwm;
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
