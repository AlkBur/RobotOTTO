#pragma once

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

enum ServoType
{
	SERVO_LEG, // нога
	SERVO_FOOT // стопа
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

	// Инициализация (ставит угол 90°)
	void begin();

	// Установить угол (0-180°)
	void setAngle(int angle);

	// Получить текущий угол
	int getAngle() const;

	// Получить тип (leg/foot)
	ServoType getType() const;

	// Получить имя типа в виде строки
	const char *getTypeName() const;

	// Установить stop
	void setStop();
	// Установка скорости мотора (-255 .. 255)
	// Положительное значение = одно направление, отрицательное = другое
	void setSpeed(int speed);

private:
	Adafruit_PWMServoDriver &_pwmDriver; // Ссылка на драйвер
	uint8_t _channel;					 // Канал PCA9685 (0-15)
	ServoType _type;
	uint16_t _minPulse;
	uint16_t _maxPulse;
	int _currentAngle;
	uint16_t _minStopPulse;
	uint16_t _maxStopPulse;

	// Преобразование угла в длину импульса
	uint16_t angleToPulse(int angle);
};
