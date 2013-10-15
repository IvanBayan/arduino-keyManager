#include "Arduino.h"
#include "keyManager.h"

keyManager::keyManager( const uint8_t keyPin, const bool eventDriven )
{
    // Configure pin for input
    pinMode(keyPin, INPUT);
    // Save the pin
    _Pin = keyPin;
    // Setup key input
    if( eventDriven ) {
        _State = _keyManagerEventDriven;
    } else {
        _State = 0;
    }
    // Clear timers
    _timerDebounce          = 0;
    _timerLongPress         = 0;
    _timerLongPressRepeat   = 0;
}


void keyManager::process() {    
    // Считывается состояние кнопки
    uint8_t key_input;
    key_input = digitalRead( _Pin );
    // Игнорируем флаги событий, выбираем флаги состояния
    switch( _State & _keyManagerStatesMask  ) {
        
        // Кнопка не нажата
        case _keyManagerReleased:
        // Если кнопку нажали
        if( key_input == _keyManagerPressState ) {
         // Установить состояние антидребезга
         _State |= _keyManagerPressDebounce;
        }
        break;
    
        // Антидребезг
        case _keyManagerPressDebounce:
        // Если кнопку отпустили до того, как прошло время антидребезга
        if( key_input != _keyManagerPressState) {
        // Очистить состояние кнопки
            _State &= _keyManagerStatesMask;
        // Очистить таймер
            _timerDebounce = 0;
        // Если кнопка нажата и прошло время антидребезга
        } else if( _timer( _keyManagerTimerDebounce ) ) {
        // Устанавливаем флаг события, состояние нажата, очищается состояние
        // антидребезга
         _State = (_State & ~ _keyManagerPressDebounce) | _keyManagerEvent | _keyManagerPressed;
        }
        break;
    
        // Кнопка нажата
        case _keyManagerPressed:
        // Если кнопку отпустили
        if( key_input != _keyManagerPressState ) {
         // Добавляем флаг антидребезга отпускания
            _State |= _keyManagerReleaseDebounce;
         // Очищаем таймер долгого нажатия
            _timerLongPress = 0;
        } else if( _timer( _keyManagerTimerLongPress )) {
         // Если не отпустили, и нажата давно, устанавливаем флаг события состояние
         // долгого нажатия, оцищаем флаг "нажата"
            _State = (_State & ~ _keyManagerPressed ) | _keyManagerEvent | _keyManagerLongPress;
        }
        break;
        
        case _keyManagerLongPress:
        if( key_input != _keyManagerPressState ) {
            // Добавляем флаг антидребезга отпускания
            _State |= _keyManagerReleaseDebounce;
             // Очищаем таймер удержания
            _timerLongPressRepeat = 0;
           }
        if( _timer( _keyManagerTimerLongPressRepeat )) {
               _State |= _keyManagerEvent;
        }
        break;
    
        case _keyManagerPressed | _keyManagerReleaseDebounce:
        case _keyManagerLongPress | _keyManagerReleaseDebounce:
            // Если это был дребезг убрать флаг антидребезга
            if( key_input == _keyManagerPressed ) { 
                _State &= ~ _keyManagerReleaseDebounce;
                _timerDebounce = 0;
            // Если нет, то очистить состояния кнопки; кнопка отпущена
            } else if( _timer( _keyManagerTimerDebounce )) {
                _State = (_State & _keyManagerFlagsMask) | _keyManagerEvent;
                _timerLongPressRepeat = 0;
            }
        break;
    
    }
}

bool keyManager::_timer( const uint8_t _mode){
// Переменная для хранения текущего значения cur_millis
uint8_t cur_millis;
switch( _mode ) {
    // Обработка антидребезка нажатия
    case _keyManagerTimerDebounce:
        // Разрешение таймера антидребезга - 1 мс
        // Диапазон значений 0 - 255 мс
        cur_millis = (millis() & 0xFF);
        if( _timerDebounce == 0 ) {
        // Если таймер не запускался записать текущее значение таймера
        // и вернуть false
        _timerDebounce = cur_millis;
        return false;
        // Иначе если время прошло, обнулить значение таймера и вернуть true
        } else if( (cur_millis > _timerDebounce && cur_millis - _timerDebounce > _delayDebounce) ||
                 ((cur_millis < _timerDebounce &&  (255 - _timerDebounce) + cur_millis > _delayDebounce) )) {
        _timerDebounce = 0;
        return true;
        } else {
        // Иначе (если время не пришло), вернуть false
        return false;
        }
    break;
    case _keyManagerTimerLongPress:
        cur_millis = (millis() >>  _keyManagerLongPressTimeShift) & 0xFF;
        if( _timerLongPress == 0 ) {
        // Если таймер не запускался записать сдвинуть на
        // _keyManagerLongPressTimeShift бит
        // размерность 2^(_keyManagerLongPressTimeShift-1) миллисекунды
        // записать младшие 8 разрядов таймера и вернуть false
          _timerLongPress = cur_millis;
          return false;
        // Иначе если время прошло, обнулить значение таймера и вернуть true
        } else if( (cur_millis > _timerLongPress && cur_millis - _timerLongPress > _delayLongPress) ||
                   (cur_millis < _timerLongPress && (255 - _timerLongPress) + cur_millis >  _delayLongPress)) {
          _timerLongPress = 0;
          return true;
        } else {
        // Иначе (если времяф не пришло), вернуть false
          return false;
        }
    break;
    case _keyManagerTimerLongPressRepeat:
        cur_millis = (millis() >> _keyManagerLongPressRepeatTimeShift ) & 0xFF;
        if( _timerLongPressRepeat == 0 ) {
          _timerLongPressRepeat = cur_millis;
          return false;
        } else if( (cur_millis  > _timerLongPressRepeat && cur_millis  - _timerLongPressRepeat > _delayLongPressRepeat) ||
                   (cur_millis  < _timerLongPressRepeat && (255 - _timerLongPressRepeat) + cur_millis >  _delayLongPressRepeat) ) {
                      _timerLongPressRepeat = cur_millis;
                      return true;
        } else {
          return false;
        }
    break;
}
}

// Возвращает true если было событие
bool keyManager::isEvent() {
    // Если флаг события установлен, вернуть true
    return _State & _keyManagerEvent;
}

bool keyManager::isReleased() {
    // Release state is 0x00, so if  we must invert result if state id 0
    if(  ! (_State & _keyManagerStatesMask ) ) {
        if( !( _State & _keyManagerEventDriven )) {
            return true;
        } else if( _State & _keyManagerEvent ) {
            _State &= ~ _keyManagerEvent;
            return true;
        } else {
            // Вернуть true
            return false;
        }
    } else {
        // Иначе false
        return false;
    }
    
}

bool keyManager::isPressed() {
    // если кнопка нажата и не событийный режим вернуть true
    if( _State & _keyManagerPressed ) {
        if( !( _State & _keyManagerEventDriven )) {
            return true;
        // если событийный режим и есть событие, сбросить событие, вернуть true
        } else if( _State & _keyManagerEvent ) {
            _State &= ~ _keyManagerEvent;
            return true;
        } else {
            // если србытия нет вернуть 
            return false;
        }
    } else {
        // если кнопка не нажата
        return false;
    }
    
}

bool keyManager::isLongPressed() {
    if( _State & _keyManagerLongPress ) {
        if( !( _State & _keyManagerEventDriven )) {
            return true;
        } else if( _State & _keyManagerEvent ) {
            _State &= ~ _keyManagerEvent;
            return true;
        } else {
            // Вернуть true
            return false;
        }
    } else {
        // Иначе false
        return false;
    }
}
void keyManager::clearEvent() {
    _State &= _keyManagerEvent;
}
