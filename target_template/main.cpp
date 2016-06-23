#include <Configuration.hpp>
#include <Module.hpp>

// --- MESSAGES ---------------------------------------------------------------
#include <common_msgs/Led.hpp>
#include <actuator_msgs/Setpoint_f32.hpp>

// --- NODES ------------------------------------------------------------------
#include <sensor_publisher/Publisher.hpp>
#include <actuator_subscriber/Subscriber.hpp>
#include <led/Subscriber.hpp>

// --- BOARD IMPL -------------------------------------------------------------
#include <QEI_driver/QEI.hpp>
#include <A4957_driver/A4957.hpp>

// *** DO NOT MOVE ***
Module module;

// --- TYPES ------------------------------------------------------------------
using QEI_Publisher  = sensor_publisher::Publisher<Configuration::QEI_DELTA_DATATYPE>;
using PWM_Subscriber = actuator_subscriber::Subscriber<float, actuator_msgs::Setpoint_f32>;

// --- NODES ------------------------------------------------------------------
led::Subscriber led_subscriber("led_subscriber", Core::MW::Thread::PriorityEnum::LOWEST);

QEI_Publisher  encoder("encoder", module.qei, Core::MW::Thread::PriorityEnum::NORMAL);
PWM_Subscriber motor("actuator_sub", module.pwm, Core::MW::Thread::PriorityEnum::NORMAL);

// --- MAIN -------------------------------------------------------------------
extern "C" {
   int
   main()
   {
      module.initialize();

      // Module configuration
      module.qei.configuration["period"] = 50;
      module.qei.configuration["ticks"]  = 1000;

      // Nodes configuration
      led_subscriber.configuration["topic"] = "led";
      encoder.configuration["topic"]        = "encoder";
      motor.configuration["topic"]          = "pwm";

      // Add nodes to the node manager (== board)...
      module.add(led_subscriber);
      module.add(encoder);
      module.add(motor);

      // ... and let's play!
      module.setup();
      module.run();

      // Is everything going well?
      for (;;) {
         if (!module.isOk()) {
            module.halt("This must not happen!");
         }

         Core::MW::Thread::sleep(Core::MW::Time::ms(500));
      }

      return Core::MW::Thread::OK;
   } // main
}
