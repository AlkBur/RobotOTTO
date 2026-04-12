#include "ServoController.h"
#include "Logger.h"

ServoController::ServoController(Adafruit_PWMServoDriver &pwmDriver, uint8_t channel,
								 ServoType type, uint16_t minPulse, uint16_t maxPulse,
								 uint16_t minStopPulse, uint16_t maxStopPulse)
	: _pwmDriver(pwmDriver), _channel(channel), _type(type),
	  _minPulse(minPulse), _maxPulse(maxPulse), _currentAngle(90),
	  _minStopPulse(minStopPulse), _maxStopPulse(maxStopPulse) {}

void ServoController::begin()
{
	setAngle(90); // Начальное положение
}

void ServoController::setAngle(int angle)
{
	if (_type == SERVO_FOOT)
	{
	}
	else
	{
		// Ограничиваем угол
		_currentAngle = constrain(angle, 0, 180);

		uint16_t pulse = angleToPulse(_currentAngle);
		LOG_DEBUG("ServoController", "Channel %d set to angle %d (PWM: %d)", _channel, _currentAngle, pulse);
		_pwmDriver.setPWM(_channel, 0, pulse);
	}
}

uint16_t ServoController::angleToPulse(int angle)
{
	// Линейное преобразование угла в длину импульса
	return map(angle, 0, 180, _minPulse, _maxPulse);
}

int ServoController::getAngle() const
{
	if (_type == SERVO_FOOT)
	{
		return 0;
	}
	return _currentAngle;
}

ServoType ServoController::getType() const
{
	return _type;
}

const char *ServoController::getTypeName() const
{
	switch (_type)
	{
	case SERVO_LEG:
		return "Leg";
	case SERVO_FOOT:
		return "Foot";
	default:
		return "Unknown";
	}
}

void ServoController::setStop()
{
	if (_type == SERVO_FOOT)
	{
		int dedta = (_maxStopPulse - _minStopPulse) / 2;
		_pwmDriver.setPWM(_channel, 0, _minStopPulse + dedta);
	}
}

void ServoController::setSpeed(int speed)
{
	if (_type != SERVO_FOOT)
	{
		return;
	}

	// Ограничиваем скорость в диапазоне [-100, 100]
	speed = constrain(speed, -100, 100);

	// Устанавливаем направление в зависимости от знака speed
	if (speed > -3 && speed < 3)
	{
		// Мертвая зона
		setStop();
	}
	else
	{
		// Преобразуем скорость (0-255) в значение ШИМ для PCA9685 (0-4095)
		uint16_t pwmValue = 0;
		if (speed >= 0)
		{
			uint16_t delta = _maxPulse - _maxStopPulse;
			pwmValue = _maxStopPulse + map(speed, 0, 100, 0, delta);
		}
		else
		{
			uint16_t delta = _minStopPulse - _minPulse;
			pwmValue = _minStopPulse - map(-speed, 0, 100, 0, delta);
		}
		_pwmDriver.setPWM(_channel, 0, pwmValue);
	}
}
