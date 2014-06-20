#include "Leds.h"

#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
#include "utils.h"
#include "SlowTicker.h"
#include "Pauser.h"
#include "modules/robot/Conveyor.h"

#include "wait_api.h"

#define enable_checksum         CHECKSUM("enable")
#define leds_disable_checksum   CHECKSUM("leds_disable")

#define pins_post_checksum      CHECKSUM("pins_post")
#define pin_gcode_checksum      CHECKSUM("pin_gcode")
#define pin_main_checksum       CHECKSUM("pin_main")
#define pin_idle_checksum       CHECKSUM("pin_idle")
#define pin_sdok_checksum       CHECKSUM("pin_sdok")
#define pin_play_checksum       CHECKSUM("pin_play")
#define mode_main_checksum      CHECKSUM("mode_main")
#define mode_idle_checksum      CHECKSUM("mode_idle")
#define blink_checksum          CHECKSUM("blink")
#define dimmed_checksum         CHECKSUM("dimmed")

#define MODE_BLINK              'b'
#define MODE_DIMMED             'd'

#define PIN_LED1                "1.18!"
#define PIN_LED2                "1.19!"
#define PIN_LED3                "1.20!"
#define PIN_LED4                "1.21!"
#define PIN_LED5                "4.28!"

#define PINS_POST_DEFAULT       PIN_LED1 "," PIN_LED2 "," PIN_LED3 "," PIN_LED4 "," PIN_LED5

Leds::Leds()  {}
Leds::~Leds() {}

void Leds::on_module_loaded()
{
    // old config leds_disable
    bool use_leds = ! THEKERNEL->config->value( leds_disable_checksum )->by_default(false)->as_bool();
         use_leds =   THEKERNEL->config->value( leds_checksum, enable_checksum )->by_default( use_leds )->as_bool();

    // Exit if this module is not enabled
    if ( ! use_leds ) {
        delete this;
        return;
    }

    // Settings
    this->on_config_reload(this);

    // register events after initializing scheme
    register_for_event(ON_MAIN_INIT);
    register_for_event(ON_SD_OK);
    register_for_event(ON_MAIN_LOOP);
    register_for_event(ON_IDLE);
    register_for_event(ON_GCODE_RECEIVED);

    THEKERNEL->slow_ticker->attach(4, this, &Leds::half_second_tick);
}

void Leds::on_config_reload(void* argument)
{
    #define CONFIG_PIN(name,default)                                            \
      pin_##name.from_string(THEKERNEL->config                                  \
                  ->value( pin_##name##_checksum, pin_##name##_checksum )       \
                  ->by_default(default)->as_string())                           \
                  ->as_output()                                                 \
                  ->set(0)

    CONFIG_PIN(gcode, PIN_LED1);
    CONFIG_PIN(main,  PIN_LED2);
    CONFIG_PIN(idle,  PIN_LED3);
    CONFIG_PIN(sdok,  PIN_LED4);
    CONFIG_PIN(play,  PIN_LED5);

    string mode;
    mode = THEKERNEL->config->value( leds_checksum, mode_main_checksum  )->by_default("blink")->as_string();
    if(mode == "blink")
        mode_main = MODE_BLINK;
    else
    if(mode == "dimmed")
        mode_main = MODE_DIMMED;
    mode = THEKERNEL->config->value( leds_checksum, mode_idle_checksum  )->by_default("blink")->as_string();
    if(mode == "blink")
        mode_idle = MODE_BLINK;
    else
    if(mode == "dimmed")
        mode_idle = MODE_DIMMED;

    counter_gcode = 0;
    counter_main  = 0;
    counter_idle  = 0;
}

#include "StreamOutputPool.h"

void Leds::on_main_init(void* argument)         {
    if(argument) {
        int& post = *(int*)argument;
        if(post != 99)
            return;
THEKERNEL->config->config_cache_load();
THEKERNEL->streams->printf("pins_post: '%s' (default)\n", PINS_POST_DEFAULT);
        // cpu time doesn't matter while booting
        // scan comma separated config string for pin descriptions
        string pins_post = THEKERNEL->config
                              ->value( leds_checksum, pins_post_checksum )
                              ->by_default(PINS_POST_DEFAULT)
                              ->as_string();
        int16_t mask = 1;
THEKERNEL->streams->printf("pins_post: '%s'\n", pins_post.c_str());
        while(pins_post.length()
              && ! (mask & 0x100) // prevent endless loop
              ) {
          Pin pin;
          string pin_bit;
          size_t comma = pins_post.find_first_of(',');
          if(comma <= pins_post.length())
            {
            pin_bit = pins_post.substr(0, comma);
            pins_post = pins_post.substr(comma+1);
            }
          else
            {
            pin_bit = pins_post;
            pins_post = "";
            }
THEKERNEL->streams->printf("post: pin_bit: '%s' pins_post: '%s' mask: %02X\n", pin_bit.c_str(), pins_post.c_str(), (int)mask);
//break;
          pin.from_string(pin_bit)->as_output();
          pin.set(post & mask);
          mask = (mask << 1);
          wait_ms(250);
        }
    }
}

void Leds::on_sd_ok(void* argument)             {
    if(pin_sdok.connected())
        pin_sdok.set(1);
}

void Leds::on_main_loop(void* argument)         {
    if(pin_main.connected()) {
        if(mode_main == MODE_BLINK)
            pin_main.set( (counter_main++ & 0x1000) ? 1 : 0 );
        else
        if(mode_main == MODE_DIMMED)
            pin_main.set( (counter_main++ & 0x0380) ? 0 : 1 );
    }
}

void Leds::on_idle(void* argument)              {
    if(pin_idle.connected()) {
        if(mode_idle == MODE_BLINK)
            pin_idle.set( (counter_idle++ & 0x1000) ? 1 : 0 );
        else
        if(mode_idle == MODE_DIMMED)
            pin_idle.set( (counter_main++ & 0x0380) ? 0 : 1 );
    }
    if(pin_gcode.connected()) {
        if(counter_gcode > 0) {
            counter_gcode++;
            if(counter_gcode > 0x0400) {
                counter_gcode = 0;
                pin_gcode.set(0);
            }
        }
    }
}

void Leds::on_gcode_received(void* argument)    {
    if(pin_gcode.connected()) {
        counter_gcode = 1;
        pin_gcode.set(1);
    }
}

uint32_t Leds::half_second_tick(uint32_t)
{
    if(pin_play.connected()) {
        if (THEKERNEL->pauser->paused())
            pin_play.set( ! pin_play.get() );
        else
            pin_play.set( ! THEKERNEL->conveyor->is_queue_empty() );
    }
    return 0;
}
