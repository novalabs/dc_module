#include <ModuleConfiguration.hpp>
#include <Module.hpp>

// --- MESSAGES ---------------------------------------------------------------
#include <core/common_msgs/Led.hpp>
#include <core/actuator_msgs/Setpoint_f32.hpp>

// --- NODES ------------------------------------------------------------------
#include <core/sensor_publisher/Publisher.hpp>
#include <core/actuator_subscriber/Subscriber.hpp>
#include <core/led/Subscriber.hpp>

// --- BOARD IMPL -------------------------------------------------------------
#include <core/QEI_driver/QEI.hpp>
#include <core/A4957_driver/A4957.hpp>

// *** DO NOT MOVE ***
Module module;

// --- TYPES ------------------------------------------------------------------
using QEI_Publisher  = core::sensor_publisher::Publisher<ModuleConfiguration::QEI_DELTA_DATATYPE>;
using PWM_Subscriber = core::actuator_subscriber::Subscriber<float, core::actuator_msgs::Setpoint_f32>;

// --- NODES ------------------------------------------------------------------
core::led::Subscriber led_subscriber("led_subscriber", core::os::Thread::PriorityEnum::LOWEST);

QEI_Publisher  encoder("encoder", module.qei, core::os::Thread::PriorityEnum::NORMAL);
PWM_Subscriber motor("actuator_sub", module.hbridge_pwm, core::os::Thread::PriorityEnum::NORMAL);

// --- MAIN -------------------------------------------------------------------
extern "C" {
   int
   main()
   {
      module.initialize();

      // Add nodes to the node manager (== board)...
      module.add(led_subscriber);
      module.add(encoder);
      module.add(motor);

      // Module configuration
      core::QEI_driver::QEI_DeltaConfiguration qei_configuration;
      qei_configuration.period = 50;
      qei_configuration.ticks  = 1000;
      module.qei.setConfiguration(qei_configuration);

      // Nodes configuration
      core::led::SubscriberConfiguration led_subscriber_configuration;
      led_subscriber_configuration.topic = "led";
      led_subscriber.setConfiguration(led_subscriber_configuration);

      core::actuator_subscriber::Configuration motor_configuration;
      motor_configuration.topic = "pwm";
      motor.setConfiguration(motor_configuration);

      core::sensor_publisher::Configuration encoder_configuration;
      encoder_configuration.topic = "encoder";
      encoder.setConfiguration(encoder_configuration);

      // ... and let's play!
      module.setup();
      module.run();

      // Is everything going well?
      for (;;) {
         if (!module.isOk()) {
            module.halt("This must not happen!");
         }

         module.keepAlive();

         core::os::Thread::sleep(core::os::Time::ms(500));
      }

      return core::os::Thread::OK;
   } // main
}
