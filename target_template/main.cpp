/* DC Module template file
 *
 */

#include <ModuleConfiguration.hpp>
#include <Module.hpp>

// --- BOARD IMPL -------------------------------------------------------------
#include <core/hw/PWM.hpp>
#include <core/QEI_driver/QEI.hpp>
#include <core/A4957_driver/A4957.hpp>

// --- MODULE -----------------------------------------------------------------
Module module;

// *** DO NOT MOVE THE CODE ABOVE THIS COMMENT *** //

// --- MESSAGES ---------------------------------------------------------------
#include <core/common_msgs/Led.hpp>
#include <core/actuator_msgs/Setpoint_f32.hpp>

// --- NODES ------------------------------------------------------------------
#include <core/sensor_publisher/Publisher.hpp>
#include <core/actuator_subscriber/Subscriber.hpp>
#include <core/led/Subscriber.hpp>

// --- TYPES ------------------------------------------------------------------
using QEI_Publisher  = core::sensor_publisher::Publisher<ModuleConfiguration::QEI_DELTA_DATATYPE>;
using PWM_Subscriber = core::actuator_subscriber::Subscriber<float, core::actuator_msgs::Setpoint_f32>;

// --- CONFIGURATIONS ---------------------------------------------------------
core::led::SubscriberConfiguration       led_subscriber_configuration_default;
core::QEI_driver::QEI_DeltaConfiguration encoder_configuration_default;
QEI_Publisher::ConfigurationType         encoder_publisher_configuration_default;
PWM_Subscriber::ConfigurationType        pwm_subscriber_configuration_default;

// --- NODES ------------------------------------------------------------------
core::led::Subscriber led_subscriber("led_sub", core::os::Thread::PriorityEnum::LOWEST);
QEI_Publisher         encoder_publisher("enc_pub", module.encoder, core::os::Thread::PriorityEnum::NORMAL);
PWM_Subscriber        motor_subscriber("pwm_sub", module.h_bridge, core::os::Thread::PriorityEnum::NORMAL);

// --- DEVICE CONFIGURATION ---------------------------------------------------
QEIConfig qei_config = {
    QEI_MODE_QUADRATURE, QEI_BOTH_EDGES, QEI_DIRINV_FALSE,
};

PWMConfig pwm_configuration = {
    72000000,/* 72MHz PWM clock.   */
    4096,   /* 12-bit PWM, 18KHz frequency. */
    nullptr,
    {
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH,NULL},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH,NULL},
        {PWM_OUTPUT_DISABLED,NULL},
        {PWM_OUTPUT_DISABLED,NULL}
    },
    0, 72
};

// --- MAIN -------------------------------------------------------------------
extern "C" {
    int
    main()
    {
        module.initialize();

        // Device configurations
        module.qei.start(qei_config);
        module.pwm.start(pwm_configuration);

        // Default configuration
        encoder_configuration_default.period = 50;
        encoder_configuration_default.ticks  = 1024;
        encoder_configuration_default.invert = 0;
        led_subscriber_configuration_default.topic    = "led";
        encoder_publisher_configuration_default.topic = "encoder";
        pwm_subscriber_configuration_default.topic    = "pwm";

        // Add configurable objects to the configuration manager...
        module.configurations().add(module.encoder, encoder_configuration_default);
        module.configurations().add(led_subscriber, led_subscriber_configuration_default);
        module.configurations().add(encoder_publisher, encoder_publisher_configuration_default);
        module.configurations().add(motor_subscriber, pwm_subscriber_configuration_default);

        // ... and load the configuration
        module.configurations().loadFrom(module.configurationStorage());

        // Add nodes to the node manager...
        module.nodes().add(led_subscriber);
        module.nodes().add(encoder_publisher);
        module.nodes().add(motor_subscriber);

        // ... and let's play!
        module.nodes().setup();
        module.nodes().run();

        // Is everything going well?
        for (;;) {
            if (!module.nodes().areOk()) {
                module.halt("This must not happen!");
            }

            core::os::Thread::sleep(core::os::Time::ms(500));

            // Remember to feed the (watch)dog!
            module.keepAlive();
        }

        return core::os::Thread::OK;
    } // main
}
