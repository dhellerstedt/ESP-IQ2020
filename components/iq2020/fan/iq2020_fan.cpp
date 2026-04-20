#include "esphome/core/log.h"
#include "iq2020_fan.h"
#include "../iq2020.h"

extern IQ2020Component* g_iq2020_main;
extern esphome::iq2020_fan::IQ2020Fan* g_iq2020_fan[FANCOUNT];

namespace esphome {
namespace iq2020_fan {

    static const char *TAG = "iq2020_fan.fan";

    void IQ2020Fan::setup() {
        if (fan_id < FANCOUNT) { g_iq2020_fan[fan_id] = this; }
        state = false;
        speed = 1;
    }

    void IQ2020Fan::dump_config() {
        ESP_LOGCONFIG(TAG, "IQ2020 fan %d config. State: %s, Speed: %d", fan_id, state ? "ON" : "OFF", speed);
    }

    esphome::fan::FanTraits IQ2020Fan::get_traits() {
        auto traits = fan::FanTraits(false, true, false, fan_speeds);
        return traits;
    };

    void IQ2020Fan::control(const esphome::fan::FanCall &call) {
        // Safe logging using value_or to avoid dereferencing empty optionals
        ESP_LOGD(TAG, "IQ2020 fan %d control. Requested State: %d, Requested Speed: %d", 
                 fan_id, 
                 call.get_state().value_or(state), 
                 call.get_speed().value_or(speed));

        // 1. Update internal state if provided
        if (call.get_state().has_value()) {
            state = *call.get_state();
        }

        // 2. Update internal speed if provided
        if (call.get_speed().has_value()) {
            speed = *call.get_speed();
        }

        int xstate = 0; // Default to OFF

        // 3. Logic to determine the command sent to the spa controller
        if (state) {
            // Use the requested speed or fall back to current speed
            xstate = speed;

            // Handle Single-Speed Jets (Jets 2)
            // If the device only supports 1 speed, an 'ON' command usually requires sending '2' (Full)
            if (fan_speeds == 1 && xstate == 1) {
                xstate = 2;
            }
        } else {
            xstate = 0; // Force OFF
        }

        // 4. Send the command to the main component
        g_iq2020_main->switchAction(fan_id + SWITCH_JETS1, xstate);
        
        // 5. Tell Home Assistant the new state
        this->publish_state();
    };

    void IQ2020Fan::updateState(int s) {
        ESP_LOGD(TAG, "IQ2020 fan %d update from controller. Raw State: %d", fan_id, s);
        switch (s) {
            case 0: // OFF
                state = false; 
                speed = 0; 
                break; 
            case 1: // LOW / MEDIUM
                state = true; 
                speed = 1; 
                break; 
            case 2: // FULL
                state = true; 
                speed = (fan_speeds == 2) ? 2 : 1; 
                break; 
        }
        this->publish_state();
    }

}  // namespace iq2020_fan
}  // namespace esphome
