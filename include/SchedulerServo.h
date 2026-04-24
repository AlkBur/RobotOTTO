// SchedulerServo.h
#pragma once

#include <Arduino.h>
#include <vector>
#include "ServoController.h"
#include "Logger.h"

struct ServoAction
{
	ServoController *servo;
	unsigned int targetAngle;
	unsigned int startAngle;
	float speed;
	bool wait;
	bool finished;
	unsigned long startTime;
	unsigned long durationMs;
	bool endless;
	FootMotionProfile footProfile;
};

class SchedulerServo
{
public:
	SchedulerServo();
	~SchedulerServo();

	void add(ServoController *servo, int targetAngle, float speedDegPerSec, bool wait = false);
	void start();
	void update();
	void stop();
	bool isDone() const;

private:
	std::vector<ServoAction> _actions;
	bool _started;

	void setServoAngle(ServoController *servo, int angle);
	unsigned int getServoAngle(ServoController *servo) const;
	int getNewAngle(const ServoAction *act, unsigned long now) const;
};
