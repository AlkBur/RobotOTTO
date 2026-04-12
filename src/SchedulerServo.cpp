#include "SchedulerServo.h"

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
	ServoAction act;
	act.servo = servo;
	act.startAngle = getServoAngle(servo); // фиксируем начальный угол СЕЙЧАС
	act.targetAngle = constrain(targetAngle, 0, 180);
	act.speed = speed;
	act.wait = wait;
	act.finished = false;
	// act.currentAngle = act.startAngle; // текущий угол = начальному
	act.startTime = 0;
	_actions.push_back(act);

	LOG_INFO("SchedulerServo", "add: servo=%p, startAngle=%d, target=%d, speed=%.1f, wait=%d",
			 servo, act.startAngle, act.targetAngle, speed, wait);
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
	// LOG_INFO("SchedulerServo", "=== UPDATE ===");

	unsigned long now = millis();
	bool anyActive = false;

	for (size_t i = 0; i < _actions.size(); ++i)
	{
		auto &act = _actions[i];
		if (act.finished)
			continue;
		anyActive = true;

		// Проверка ожидания
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

		// Первый запуск
		if (act.startTime == 0)
		{
			act.startTime = now;
			// act.currentAngle = act.startAngle;
			LOG_DEBUG("SchedulerServo", "action[%d] start: startAngle=%d, target=%d, speed=%.1f",
					  i, act.startAngle, act.targetAngle, act.speed);
		}

		// Мгновенное действие
		if (act.speed <= 0)
		{
			act.servo->setAngle(act.targetAngle);
			// act.currentAngle = act.targetAngle;
			act.finished = true;
			LOG_INFO("SchedulerServo", "action[%d] instant done -> angle %d", i, act.targetAngle);
			continue;
		}

		// Плавное движение
		int currentAngle = getServoAngle(act.servo);
		int newAngle = getNewAngle(&act, now);
		// LOG_DEBUG("SchedulerServo", "action[%d] newAngle=%d, current=%d, target=%d",
		// 			i, newAngle, act.currentAngle, act.targetAngle);
		if (newAngle != currentAngle)
		{

			// act.currentAngle = newAngle;
			act.servo->setAngle(newAngle);
			LOG_DEBUG("SchedulerServo", "action[%d] setAngle: %d currentAngle: %d", i, newAngle, currentAngle);
		}

		// Завершение
		if (newAngle == static_cast<int>(act.targetAngle))
		{
			act.finished = true;
			LOG_INFO("SchedulerServo", "action[%d] finished at angle %d", i, newAngle);
		}
	}

	// if (!anyActive && !_actions.empty())
	// {
	// 	LOG_DEBUG("SchedulerServo", "All actions finished");
	// }
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

// --- Адаптация под ваш класс серво (ServoController) ---
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

	int startAngle = constrain(static_cast<int>(act->startAngle), 0, 180);
	int targetAngle = constrain(static_cast<int>(act->targetAngle), 0, 180);

	if (startAngle == targetAngle)
		return targetAngle;

	int direction = (targetAngle > startAngle) ? 1 : -1;
	unsigned long delta = now - act->startTime;
	float traveledFloat = (delta / 1000.0f) * act->speed; // act->speed в градусах в секунду
	int traveled = static_cast<int>(traveledFloat);
	int newAngle = startAngle + direction * traveled;

	// Ограничение целевым углом
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
