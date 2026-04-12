// SchedulerServo.h
#pragma once

#include <Arduino.h>
#include <vector>
#include "ServoController.h"
#include "Logger.h"

struct ServoAction
{
	ServoController *servo;	  // указатель на объект серво
	unsigned int targetAngle; // целевой угол 0-180
	unsigned int startAngle;  // целевой угол 0-180
	float speed;			  // градусов в секунду (0 = мгновенно)
	bool wait;				  // ожидать завершения всех предыдущих
	bool finished;			  // закончил выполняться
	// int currentAngle;		  // текущий угол при выполнении (заполняется при активации)
	unsigned long startTime; // время запуска движения (ms)
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
