#include "SchedulerServo.h"

namespace
{
unsigned long calculateFootDurationMs(unsigned int targetAngle, float actualAngularSpeedDegPerSec)
{
	const float absSpeed = fabsf(actualAngularSpeedDegPerSec);
	if (targetAngle == 0 || absSpeed <= 0.0f)
	{
		return 0;
	}

	const float durationMs = (static_cast<float>(targetAngle) / absSpeed) * 1000.0f;
	return static_cast<unsigned long>(ceilf(durationMs));
}
} // namespace

SchedulerServo::SchedulerServo() : _started(false)
{
	LOG_DEBUG("SchedulerServo", "Constructor called");
}

SchedulerServo::~SchedulerServo()
{
	LOG_DEBUG("SchedulerServo", "Destructor called");
	stop();
}

void SchedulerServo::add(ServoController *servo, int targetAngle, float speed, bool wait)
{
	if (servo == nullptr)
	{
		LOG_WARNING("SchedulerServo", "add() skipped: servo=nullptr");
		return;
	}

	ServoAction act{};
	act.servo = servo;
	act.wait = wait;
	act.finished = false;
	act.startTime = 0;
	act.durationMs = 0;
	act.endless = false;
	act.footProfile = {0, 0.0f};

	if (servo->isFoot())
	{
		act.startAngle = 0;
		act.targetAngle = static_cast<unsigned int>(max(targetAngle, 0));
		act.speed = speed;
		act.endless = (act.targetAngle == 0);

		if (fabsf(speed) <= 0.0f)
		{
			LOG_WARNING("SchedulerServo", "add(): Foot requires non-zero speed, target=%u", act.targetAngle);
			act.finished = true;
		}
		else if (!servo->selectFootProfile(speed, act.footProfile))
		{
			LOG_WARNING("SchedulerServo", "add(): Foot profile not found for speed %.1f", speed);
			act.finished = true;
		}
		else if (!act.endless)
		{
			act.durationMs = calculateFootDurationMs(act.targetAngle, act.footProfile.angularSpeedDegPerSec);
		}

		LOG_INFO("SchedulerServo", "add Foot: servo=%p, angle=%u, requestedSpeed=%.1f, actualSpeed=%.1f, pulse=%u, duration=%lu, endless=%d, wait=%d",
				 servo, act.targetAngle, speed, act.footProfile.angularSpeedDegPerSec, act.footProfile.pulse,
				 act.durationMs, act.endless, wait);
	}
	else
	{
		act.startAngle = 0;
		act.targetAngle = constrain(targetAngle, 0, 180);
		act.speed = speed;

		LOG_INFO("SchedulerServo", "add Leg: servo=%p, target=%d, speed=%.1f, wait=%d",
				 servo, act.targetAngle, speed, wait);
	}

	_actions.push_back(act);
}

void SchedulerServo::start()
{
	if (_actions.empty())
	{
		LOG_WARNING("SchedulerServo", "start() called but no actions");
		return;
	}
	_started = true;
	LOG_INFO("SchedulerServo", "start() called, _started=true, actions count=%d", _actions.size());
}

void SchedulerServo::update()
{
	LOG_VERBOSE("SchedulerServo", "update() entered, _started=%d", _started);

	if (!_started)
	{
		LOG_VERBOSE("SchedulerServo", "update() exited: _started=false");
		return;
	}

	const unsigned long now = millis();
	bool anyActive = false;

	for (size_t i = 0; i < _actions.size(); ++i)
	{
		auto &act = _actions[i];
		if (act.finished)
		{
			continue;
		}
		anyActive = true;

		if (act.wait)
		{
			bool prevFinished = true;
			for (size_t j = 0; j < i; ++j)
			{
				if (!_actions[j].finished)
				{
					prevFinished = false;
					break;
				}
			}
			if (!prevFinished)
			{
				LOG_VERBOSE("SchedulerServo", "action[%d] waiting for previous", i);
				continue;
			}
		}

		if (act.startTime == 0)
		{
			act.startTime = now;
			if (act.servo->isFoot())
			{
				act.servo->setFootPulse(act.footProfile.pulse);
				LOG_DEBUG("SchedulerServo", "action[%d] Foot start: pulse=%u, speed=%.1f, duration=%lu, endless=%d",
						  i, act.footProfile.pulse, act.footProfile.angularSpeedDegPerSec, act.durationMs, act.endless);
			}
			else
			{
				act.startAngle = getServoAngle(act.servo);
				LOG_DEBUG("SchedulerServo", "action[%d] Leg start: startAngle=%d, target=%d, speed=%.1f",
						  i, act.startAngle, act.targetAngle, act.speed);
			}
		}

		if (act.servo->isFoot())
		{
			if (act.endless)
			{
				continue;
			}

			const unsigned long elapsed = now - act.startTime;
			if (elapsed >= act.durationMs)
			{
				act.servo->setStop();
				act.finished = true;
				LOG_INFO("SchedulerServo", "action[%d] Foot finished after %lu ms", i, elapsed);
			}
			continue;
		}

		if (act.speed <= 0)
		{
			act.servo->setAngle(act.targetAngle);
			act.finished = true;
			LOG_INFO("SchedulerServo", "action[%d] instant done -> angle %d", i, act.targetAngle);
			continue;
		}

		const int currentAngle = getServoAngle(act.servo);
		const int newAngle = getNewAngle(&act, now);
		if (newAngle != currentAngle)
		{
			act.servo->setAngle(newAngle);
			LOG_DEBUG("SchedulerServo", "action[%d] setAngle: %d currentAngle: %d", i, newAngle, currentAngle);
		}

		if (newAngle == static_cast<int>(act.targetAngle))
		{
			act.finished = true;
			LOG_INFO("SchedulerServo", "action[%d] finished at angle %d", i, newAngle);
		}
	}

	if (!anyActive && !_actions.empty())
	{
		_started = false;
		_actions.clear();
		LOG_INFO("SchedulerServo", "all actions finished, scheduler stopped");
	}

	LOG_VERBOSE("SchedulerServo", "update() finished");
}

void SchedulerServo::stop()
{
	for (auto &act : _actions)
	{
		if (act.servo != nullptr && act.servo->isFoot())
		{
			act.servo->setStop();
		}
	}

	_started = false;
	_actions.clear();
	LOG_INFO("SchedulerServo", "stop() called, cleared all actions");
}

bool SchedulerServo::isDone() const
{
	if (!_started)
		return true;
	for (const auto &act : _actions)
	{
		if (!act.finished)
			return false;
	}
	return true;
}

void SchedulerServo::setServoAngle(ServoController *servo, int angle)
{
	servo->setAngle(angle);
}

unsigned int SchedulerServo::getServoAngle(ServoController *servo) const
{
	unsigned int angle = servo->getAngle();
	LOG_VERBOSE("SchedulerServo", "getServoAngle: servo=%p, angle=%d", servo, angle);
	return angle;
}

int SchedulerServo::getNewAngle(const ServoAction *act, unsigned long now) const
{
	if (act->speed <= 0)
		return act->targetAngle;

	const int startAngle = constrain(static_cast<int>(act->startAngle), 0, 180);
	const int targetAngle = constrain(static_cast<int>(act->targetAngle), 0, 180);

	if (startAngle == targetAngle)
		return targetAngle;

	const int direction = (targetAngle > startAngle) ? 1 : -1;
	const unsigned long delta = now - act->startTime;
	const float traveledFloat = (delta / 1000.0f) * act->speed;
	const int traveled = static_cast<int>(traveledFloat);
	int newAngle = startAngle + direction * traveled;

	if (direction == 1)
	{
		if (newAngle > targetAngle)
			newAngle = targetAngle;
	}
	else
	{
		if (newAngle < targetAngle)
			newAngle = targetAngle;
	}

	LOG_VERBOSE("SchedulerServo", "getNewAngle: delta=%lu, start=%d, traveled=%d (%.2f), new=%d, target=%d",
				delta, startAngle, traveled, traveledFloat, newAngle, targetAngle);

	return newAngle;
}
