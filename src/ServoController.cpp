#include "ServoController.h"
#include "Logger.h"

namespace
{
	struct FootCalibrationEntry
	{
		uint16_t pulse;
		float angularSpeedDegPerSec;
	};

	constexpr FootCalibrationEntry kLeftFootBackwardProfiles[] = {
		{257, 140.0f},
		{264, 110.0f},
		{272, 75.0f},
		{275, 60.0f},
		{278, 18.0f},
	};

	constexpr FootCalibrationEntry kLeftFootForwardProfiles[] = {
		// {314, 8.0f},
		// {322, 89.0f},
		// {329, 187.0f},
		// {337, 205.0f},
		{314, 8.0f},
		{316, 15.0f}, // добавлено
		{318, 25.0f}, // добавлено
		{320, 35.0f}, // добавлено
		{322, 89.0f},
		{325, 120.0f}, // добавлено
		{329, 187.0f},
		{333, 195.0f}, // добавлено
		{337, 205.0f},
	};

	constexpr FootCalibrationEntry kRightFootBackwardProfiles[] = {
		{257, 230.0f},
		{264, 173.0f},
		{272, 75.0f},
		{276, 60.0f},
		{278, 18.0f},
	};

	constexpr FootCalibrationEntry kRightFootForwardProfiles[] = {
		{312, 8.0f},
		{314, 20.0f}, // добавить
		{316, 35.0f}, // добавлено
		{318, 45.0f}, // добавлено
		{320, 53.0f},
		{321, 60.0f},  // ← новый профиль для 60°/sec
		{323, 90.0f},  // добавлено
		{325, 130.0f}, // добавлено
		{327, 189.0f},
		{331, 200.0f}, // добавлено
		{335, 208.0f},
	};

	bool findClosestProfile(const FootCalibrationEntry *profiles, size_t count, float requestedAbsSpeed, FootMotionProfile &profile)
	{
		if (profiles == nullptr || count == 0 || requestedAbsSpeed <= 0.0f)
		{
			return false;
		}

		bool foundCeil = false;
		FootCalibrationEntry selected = profiles[0];
		FootCalibrationEntry maxProfile = profiles[0];

		for (size_t i = 0; i < count; ++i)
		{
			if (profiles[i].angularSpeedDegPerSec > maxProfile.angularSpeedDegPerSec)
			{
				maxProfile = profiles[i];
			}

			if (profiles[i].angularSpeedDegPerSec >= requestedAbsSpeed)
			{
				if (!foundCeil || profiles[i].angularSpeedDegPerSec < selected.angularSpeedDegPerSec)
				{
					selected = profiles[i];
					foundCeil = true;
				}
			}
		}

		if (!foundCeil)
		{
			selected = maxProfile;
		}

		profile.pulse = selected.pulse;
		profile.angularSpeedDegPerSec = selected.angularSpeedDegPerSec;
		return true;
	}
} // namespace

ServoController::ServoController(Adafruit_PWMServoDriver &pwmDriver, uint8_t channel,
								 ServoType type, uint16_t minPulse, uint16_t maxPulse,
								 uint16_t minStopPulse, uint16_t maxStopPulse)
	: _pwmDriver(pwmDriver), _channel(channel), _type(type),
	  _minPulse(minPulse), _maxPulse(maxPulse), _currentAngle(90),
	  _minStopPulse(minStopPulse), _maxStopPulse(maxStopPulse) {}

void ServoController::begin()
{
	if (_type == SERVO_FOOT)
	{
		setStop();
		return;
	}

	setAngle(90);
}

void ServoController::setAngle(int angle)
{
	if (_type == SERVO_FOOT)
	{
		LOG_DEBUG("ServoController", "Channel %d ignored setAngle(%d) for Foot", _channel, angle);
		return;
	}

	_currentAngle = constrain(angle, 0, 180);

	uint16_t pulse = angleToPulse(_currentAngle);
	LOG_DEBUG("ServoController", "Channel %d set to angle %d (PWM: %d)", _channel, _currentAngle, pulse);
	_pwmDriver.setPWM(_channel, 0, pulse);
}

uint16_t ServoController::angleToPulse(int angle)
{
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

bool ServoController::isFoot() const
{
	return _type == SERVO_FOOT;
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
	if (_type != SERVO_FOOT)
	{
		return;
	}

	const uint16_t stopPulse = static_cast<uint16_t>((_minStopPulse + _maxStopPulse + 1) / 2);
	_pwmDriver.setPWM(_channel, 0, stopPulse);
	LOG_DEBUG("ServoController", "Channel %d set STOP pulse %d", _channel, stopPulse);
}

void ServoController::setFootPulse(uint16_t pulse)
{
	if (_type != SERVO_FOOT)
	{
		return;
	}

	_pwmDriver.setPWM(_channel, 0, pulse);
	LOG_DEBUG("ServoController", "Channel %d set foot pulse %d", _channel, pulse);
}

bool ServoController::selectFootProfile(float requestedAngularSpeedDegPerSec, FootMotionProfile &profile) const
{
	if (_type != SERVO_FOOT)
	{
		return false;
	}

	const float requestedAbsSpeed = fabsf(requestedAngularSpeedDegPerSec);
	if (requestedAbsSpeed <= 0.0f)
	{
		return false;
	}

	const bool forward = requestedAngularSpeedDegPerSec > 0.0f;
	const FootCalibrationEntry *profiles = nullptr;
	size_t count = 0;

	if (_channel == 4)
	{
		profiles = forward ? kLeftFootForwardProfiles : kLeftFootBackwardProfiles;
		count = forward ? (sizeof(kLeftFootForwardProfiles) / sizeof(kLeftFootForwardProfiles[0]))
						: (sizeof(kLeftFootBackwardProfiles) / sizeof(kLeftFootBackwardProfiles[0]));
	}
	else if (_channel == 8)
	{
		profiles = forward ? kRightFootForwardProfiles : kRightFootBackwardProfiles;
		count = forward ? (sizeof(kRightFootForwardProfiles) / sizeof(kRightFootForwardProfiles[0]))
						: (sizeof(kRightFootBackwardProfiles) / sizeof(kRightFootBackwardProfiles[0]));
	}
	else
	{
		LOG_WARNING("ServoController", "Channel %d has no Foot calibration table", _channel);
		return false;
	}

	if (!findClosestProfile(profiles, count, requestedAbsSpeed, profile))
	{
		return false;
	}

	if (!forward)
	{
		profile.angularSpeedDegPerSec = -profile.angularSpeedDegPerSec;
	}

	LOG_INFO("ServoController", "Channel %d selected Foot profile: requested=%.1f actual=%.1f pulse=%d",
			 _channel, requestedAngularSpeedDegPerSec, profile.angularSpeedDegPerSec, profile.pulse);
	return true;
}

void ServoController::setSpeed(int speed)
{
	if (_type != SERVO_FOOT)
	{
		return;
	}

	speed = constrain(speed, -100, 100);

	if (speed > -3 && speed < 3)
	{
		setStop();
	}
	else
	{
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
		LOG_DEBUG("ServoController", "Channel %d setSpeed(%d) -> PWM %d", _channel, speed, pwmValue);
	}
}
