#pragma once

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

enum ServoType
{
	SERVO_LEG, // нога
	SERVO_FOOT // стопа
};

struct FootMotionProfile
{
	uint16_t pulse;
	float angularSpeedDegPerSec;
};

class ServoController
{
public:
	// Конструктор
	// pwmDriver: ссылка на глобальный объект Adafruit_PWMServoDriver
	// channel: номер канала на PCA9685 (0-15)
	// type: тип сервопривода (нога/стопа)
	// minPulse: минимальная длина импульса для 0 градусов (по умолч. 150)
	// maxPulse: максимальная длина импульса для 180 градусов (по умолч. 600)
	ServoController(Adafruit_PWMServoDriver &pwmDriver, uint8_t channel,
					ServoType type, uint16_t minPulse = 150, uint16_t maxPulse = 600,
					uint16_t minStopPulse = 0, uint16_t maxStopPulse = 0);

	// Инициализация
	void begin();

	// Установить угол (0-180°) для позиционных сервоприводов
	void setAngle(int angle);

	// Получить текущий угол
	int getAngle() const;

	// Получить тип (leg/foot)
	ServoType getType() const;
	bool isFoot() const;

	// Получить имя типа в виде строки
	const char *getTypeName() const;

	// Остановить вращение стопы
	void setStop();

	// Установить точное PWM значение для стопы
	void setFootPulse(uint16_t pulse);

	// Подобрать профиль вращения стопы по измеренной угловой скорости
	bool selectFootProfile(float requestedAngularSpeedDegPerSec, FootMotionProfile &profile) const;

	// Установка скорости мотора (-100 .. 100)
	void setSpeed(int speed);

private:
	Adafruit_PWMServoDriver &_pwmDriver;
	uint8_t _channel;
	ServoType _type;
	uint16_t _minPulse;
	uint16_t _maxPulse;
	int _currentAngle;
	uint16_t _minStopPulse;
	uint16_t _maxStopPulse;

	uint16_t angleToPulse(int angle);
};
