# RobotOTTO

Робот-андроид с дистанционным управлением через веб-интерфейс.

![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32%20C3-orange)
![Arduino](https://img.shields.io/badge/Framework-Arduino-blue)

---

## Описание

RobotOTTO — робот-андроид с антропоморфной конструкцией, оснащённый:
- Двумя сервоприводами для управления ногами (позиционные, 180°)
- Двумя сервоприводами для колёс (непрерывного вращения, 360°)
- OLED-дисплеями для отображения эмоций на лице
- MP3-плеером для звуковых эффектов
- WiFi-точкой доступа с веб-интерфейсом управления

Управление осуществляется через браузер: джойстик, D-Pad, кнопки действий.

---

## Функциональность

- **Управление движением**
  - Дифференциальный привод (два колеса)
  - Плавное движение ног (позиционные сервоприводы)
  - Запрограммированные движения (танцы, приветствие)

- **Эмоции**
  - OLED-дисплеи 128x64 с анимированными глазами
  - Смена настроений (default, happy, angry, tired)
  - Автоматическое моргание и движение глаз

- **Звук**
  - DFPlayer Mini для MP3-аудио
  - Автоматическое воспроизведение треков

- **Интерфейс управления**
  - Веб-интерфейс (HTML/CSS/JS)
  - WebSocket для实时 управления
  - WiFi AP режим (точка доступа)

---

## Оборудование

### Детали для 3D-печати

 STL-файлы для корпуса робота доступны на [Printables.com](https://www.printables.com/model/1250758-otto-ninja-xl-remix-version) (модель Otto Ninja XL Remix).

### Основная плата

| Компонент | Модель | Примечание |
|-----------|--------|-----------|
| Микроконтроллер | ESP32-C3 SuperMini | 160 МГц, WiFi, USB-C |
| Плата расширения | Expansion Board for ESP32-C3 SuperMini | |
| Модуль питания | LiPo модуль USB Type-C | Питание от Li-Ion аккумулятора |

### Сервоприводы

| Серво | Модель | Тип | Канал PCA9685 | Min PWM | Max PWM |
|------|--------|-----|---------------|---------|---------|
| Левая нога | MG996 | Позиционный (180°) | CH5 | 120 | 500 |
| Правая нога | MG996 | Позиционный (180°) | CH9 | 130 | 520 |
| Левая стопа | MG996 | Непрерывный (360°) | CH4 | 257 | 337 |
| Правая стопа | MG996 | Непрерывный (360°) | CH8 | 257 | 335 |

### Дисплеи

| Дисплей | Модель | Адрес I2C | Канал мультиплексора |
|---------|--------|-----------|---------------------|
| Левый глаз | AITEWIN Robot OLED 1.3" | 0x3C | TCA9548A CH0 |
| Правый глаз | AITEWIN Robot OLED 1.3" | 0x3C | TCA9548A CH1 |

### Прочие модули

| Модуль | Модель | Подключение | Примечание |
|--------|--------|-------------|-----------|
| PWM-контроллер | PCA9685 | I2C (SDA=GPIO0, SCL=GPIO3) | 16-канальный, 5 В |
| Мультиплексор | TCA9548A | I2C (SDA=GPIO0, SCL=GPIO3) | I2C-мультиплексор для OLED |
| MP3-плеер | DFPlayer Mini MP3 | GPIO20 (RX), GPIO21 (TX) | UART (Serial1), MP3/WAV |

---

## Схема подключения

### I2C шина (SDA=GPIO0, SCL=GPIO3)

| Устройство | Адрес | Примечание |
|------------|-------|------------|
| PCA9685 | 0x40 | PWM-контроллер сервоприводов |
| TCA9548A | 0x70 | Мультиплексор для OLED-дисплеев |

### UART (Serial1)

| Вывод ESP32 | Вывод DFPlayer | Примечание |
|-------------|----------------|------------|
| GPIO20 | RX | Приём данных |
| GPIO21 | TX | Передача данных |

### Питание

| Компонент | Напряжение |
|-----------|------------|
| ESP32-C3 | 5 В (USB или VIN) |
| PCA9685 | 5-6 В (extern) |
| Сервоприводы | 5-6 В (extern) |
| DFPlayer | 3.3-5 В |

---

## Требования

- **PlatformIO Core** или **PlatformIO IDE** (VS Code)
- **Python 3.8+** (для PlatformIO)
- **Chrome/Firefox/Safari** (для веб-интерфейса)

### Библиотеки (автоматически через platformio.ini)

| Библиотека | Версия | Назначение |
|------------|--------|-----------|
| Adafruit PWM Servo Driver | ^3.0.3 | Управление PCA9685 |
| ESPAsyncWebServer | ^3.10.3 | Асинхронный веб-сервер |
| AsyncTCP | ^3.4.10 | Асинхронный TCP для ESP32 |
| DFRobotDFPlayerMini | ^1.0.6 | MP3-плеер |
| Adafruit BusIO | ^1.17.4 | I2C/SPI абстракция |
| FluxGarage RoboEyes | ^1.1.1 | Анимация глаз |
| Adafruit SH110X | ^2.1.14 | OLED SH1106 драйвер |

---

## Установка

### 1. Установка PlatformIO

**Windows:**
```bash
pip install platformio
```

**macOS/Linux:**
```bash
sudo pip install platformio
```

Или используйте [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) для VS Code.

### 2. Клонирование проекта

```bash
git clone https://github.com/your-repo/RobotOTTO.git
cd RobotOTTO
```

### 3. Сборка и прошивка

```bash
# Сборка
pio run

# Прошивка (автоопределение порта)
pio run --target upload

# Мониторинг Serial
pio device monitor
```

### 4. Подключение к роботу

1. После прошивки робот создаёт WiFi-точку доступа `RobotOTTO`
2. Пароль по умолчанию: `password`
3. Подключитесь к WiFi и откройте в браузере: `http://192.168.4.1`

---

## Структура проекта

```
RobotOTTO/
├── src/                    # Исходный код (C++)
│   ├── main.cpp           # Точка входа, инициализация
│   ├── ServoController.cpp # Управление сервоприводами
│   ├── SchedulerServo.cpp # Планировщик движений
│   ├── Eyes.cpp          # Управление глазами
│   ├── DFPlayer.cpp      # MP3-плеер
│   ├── WebSocketServer.cpp# WebSocket сервер
│   └── Logger.cpp        # Логирование
├── include/               # Заголовочные файлы
│   ├── ServoController.h
│   ├── SchedulerServo.h
│   ├── Eyes.h
│   ├── DFPlayer.h
│   ├── WebSocketServer.h
│   ├── Logger.h
│   └── index_html.h       # Веб-интерфейс (PROGMEM)
├── lib/                   # Локальные библиотеки
│   └── README
├── test/                  # Тесты (пусто)
├── platformio.ini         # Конфигурация сборки
├── index.html             # Веб-интерфейс (для разработки)
├── README.md              # Этот файл
└── AGENTS.md              # Инструкции для AI-агентов
```

---

## Конфигурация

### platformio.ini

```ini
[env:esp32c3]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino

; Скорость Serial-монитора
monitor_speed = 115200

; Скорость загрузки прошивки
upload_speed = 921600

; Оптимизация флеш-памяти
board_build.flash_mode = qio
board_build.f_flash = 80000000L

; Частота CPU
board_build.f_cpu = 160000000L

; Флаги компиляции
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
    -DCORE_DEBUG_LEVEL=3
    -std=gnu++17

; Зависимости (библиотеки)
lib_deps =
    adafruit/Adafruit PWM Servo Driver Library@^3.0.3
    esp32async/ESPAsyncWebServer@^3.10.3
    dfrobot/DFRobotDFPlayerMini@^1.0.6
    esp32async/AsyncTCP@^3.4.10
    adafruit/Adafruit BusIO@^1.17.4
    fluxgarage/FluxGarage RoboEyes@^1.1.1
    adafruit/Adafruit SH110X@^2.1.14
```

### Настройки WiFi (в коде)

```cpp
// В include/index_html.h
#define WIFI_SSID "ESP32-Robot"
#define WIFI_PASS "12345678"
```

---

## Протокол управления

### WebSocket (ESP ← Browser)

**Формат:** компактные строки

| Пакет | Пример | Описание |
|-------|--------|---------|
| `jX,Y` | `jL,127,127` | Джойстик (x,y от -128 до 127) |
| `dU1` / `dU0` | `dL1` | D-Pad (направление + состояние) |
| `M0` - `M3` | `M3` | Действия (крест, круг, квадрат, треугольник) |
| `PP0` / `PP1` | `PP0` | Воспроизведение/пауза |
| `V15` | `V20` | Громкость (0-30) |
| `L0,90` | `L1,45` | Установка угла ноги |

### WebSocket (ESP → Browser)

| Пакет | Пример | Описание |
|-------|--------|---------|
| `p05` | `p05` | DFPlayer тип (hex) |
| `cL150,600` | `cL150,600` | Калибровка левой ноги (min,max) |
| `cR150,600` | `cR150,600` | Калибровка правой ноги (min,max) |

---

## Калибровка сервоприводов стоп

### Измеренные характеристики (левая стопа, CH4)

Dead zone: 279–314, Stop: 297

| PWM | Направление | Угловая скорость (°/с) |
|-----|-------------|-------------------------|
| 257 | BWD max | 140 |
| 264 | BWD 2 | 110 |
| 272 | BWD 1 | 75 |
| 278 | BWD min | 18 |
| 297 | STOP | 0 |
| 314 | FWD min | 8 |
| 322 | FWD 1 | 89 |
| 329 | FWD 2 | 187 |
| 337 | FWD max | 205 |

### Измеренные характеристики (правая стопа, CH8)

Dead zone: 279–312, Stop: 296

| PWM | Направление | Угловая скорость (°/с) |
|-----|-------------|-------------------------|
| 257 | BWD max | 230 |
| 264 | BWD 2 | 173 |
| 272 | BWD 1 | 75 |
| 278 | BWD min | 18 |
| 296 | STOP | 0 |
| 312 | FWD min | 8 |
| 320 | FWD 1 | 107 |
| 327 | FWD 2 | 189 |
| 335 | FWD max | 208 |

---

## Отладка

### Serial-монитор

```bash
pio device monitor
```

Настройки:
- Скорость: 115200 baud
- Конец строки: NL + CR

### Уровни логирования

В `main.cpp`:
```cpp
Logger::setLevel(LOG_LEVEL_DEBUG);   // Все сообщения
Logger::setLevel(LOG_LEVEL_INFO);    // INFO и ERROR
Logger::setLevel(LOG_LEVEL_WARNING); // Только WARNING и ERROR
Logger::setLevel(LOG_LEVEL_ERROR);    // Только ERROR
```

### OTA (беспроводная прошивка)

```bash
pio run --target upload --upload-port 192.168.4.1
```

---

## Лицензия

MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Автор

Burlakov Alexander