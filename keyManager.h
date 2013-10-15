// Comment will be here

#ifndef keyManager_h
#define Morse_h

#include <Arduino.h>
#include <inttypes.h>

// Timers defines
#define _keyManagerDebounceTime                 2
#define _keyManagerLongPressTimeShift           6
#define _keyManagerLongPressTime                1500
#define _keyManagerLongPressRepeatTimeShift     1
#define _keyManagerLongPressRepeatTime          100

// Key states and flags
#define _keyManagerReleased             0x00
#define _keyManagerPressDebounce        0x01
#define _keyManagerPressed              0x02
#define _keyManagerLongPress            0x04
#define _keyManagerReleaseDebounce      0x08
#define _keyManagerStatesMask           0x0F

#define _keyManagerEventDriven       0x40
#define _keyManagerEvent                0x80
#define _keyManagerFlagsMask            0xF0

// Timer modes
#define _keyManagerTimerDebounce            0x00
#define _keyManagerTimerLongPress           0x01
#define _keyManagerTimerLongPressRepeat     0x02

// Key press signal (can't be HIGH with INPUT_PULLUP)
#define _keyManagerPressState       LOW

// Delays
#define _delayDebounce          _keyManagerDebounceTime
#define _delayLongPress         (_keyManagerLongPressTime/(1<<_keyManagerLongPressTimeShift))
#define _delayLongPressRepeat  (_keyManagerLongPressRepeatTime/(1<<_keyManagerLongPressRepeatTimeShift))

class keyManager {
    public:
        keyManager( const uint8_t keyPin, const bool auto_clear );
        void process();
        bool isEvent();
        bool isReleased();
        bool isPressed();
        bool isLongPressed();
        void clearEvent();
    private:
        // Key 
        uint8_t _Pin;
        uint8_t _State;
        // Timers
        uint8_t _timerDebounce, _timerLongPress, _timerLongPressRepeat;
        bool _timer( const uint8_t _mode);
        
};

#endif
