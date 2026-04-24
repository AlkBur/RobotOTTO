#include "Eyes.h"
#include "Logger.h"

// Теперь включаем библиотеку ТОЛЬКО здесь
#ifdef DEFAULT
#pragma push_macro("DEFAULT")
#undef DEFAULT
#endif

#include "FluxGarage_RoboEyes.h"

#ifdef DEFAULT
#pragma pop_macro("DEFAULT")
#endif

#ifndef DEFAULT
#define DEFAULT 0
#endif

// Конструктор
Eyes::Eyes(uint8_t multiplexerAddr, uint8_t left_channel, uint8_t right_channel)
	: _muxAddr(multiplexerAddr), _left_channel(left_channel), _right_channel(right_channel), _displayLeft(128, 64, &Wire, -1), _displayRight(128, 64, &Wire, -1), _eyeLeft(nullptr), _eyeRight(nullptr), _initialized(false), _autoMove(true), _autoBlink(true), _lastMoveTime(0), _lastBlinkTime(0), _currentMood(MOOD_DEFAULT)
{
}

bool Eyes::begin()
{
	// Проверяем наличие мультиплексора
	Wire.beginTransmission(_muxAddr);
	if (Wire.endTransmission() != 0)
	{
		LOG_ERROR("Eyes", "TCA9548A not found at address 0x%02X", _muxAddr);
		return false;
	}

	// Инициализация левого глаза
	_selectChannel(_left_channel);
	if (!_displayLeft.begin(0x3C, true))
	{
		LOG_ERROR("Eyes", "Left display init failed");
		return false;
	}
	_displayLeft.clearDisplay();
	_displayLeft.display();

	// Инициализация правого глаза
	_selectChannel(_right_channel);
	if (!_displayRight.begin(0x3C, true))
	{
		LOG_ERROR("Eyes", "Right display init failed");
		return false;
	}
	_displayRight.clearDisplay();
	_displayRight.display();

	// Создаём объекты глаз (теперь в куче)
	_eyeLeft = new RoboEyes<Adafruit_SH1106G>(_displayLeft);
	_eyeRight = new RoboEyes<Adafruit_SH1106G>(_displayRight);

	// Настройка глаз
	_selectChannel(_left_channel);
	_eyeLeft->setCyclops(true);
	_eyeLeft->begin(128, 64, 60);

	_selectChannel(_right_channel);
	_eyeRight->setCyclops(true);
	_eyeRight->begin(128, 64, 60);

	_initialized = true;
	LOG_INFO("Eyes", "Eyes initialized successfully");

	return true;
}

void Eyes::update()
{
	if (!_initialized)
		return;

	// Обновляем состояние каждого глаза
	_selectChannel(_left_channel);
	_eyeLeft->update();

	_selectChannel(_right_channel);
	_eyeRight->update();

	// Автоматическое движение и моргание
	if (_autoMove)
	{
		_updateAutoMove();
	}

	if (_autoBlink)
	{
		_updateAutoBlink();
	}
}

void Eyes::blink()
{
	if (!_initialized)
		return;

	_selectChannel(_left_channel);
	_eyeLeft->blink();

	_selectChannel(_right_channel);
	_eyeRight->blink();

	_lastBlinkTime = millis();
}

void Eyes::setAutoMove(bool enable)
{
	_autoMove = enable;
}

void Eyes::setAutoBlink(bool enable)
{
	_autoBlink = enable;
}

void Eyes::setEyePosition(int leftX, int leftY, int rightX, int rightY)
{
	if (!_initialized)
		return;

	int maxX = _eyeLeft->getScreenConstraint_X();
	int maxY = _eyeLeft->getScreenConstraint_Y();

	leftX = constrain(leftX, 0, maxX);
	leftY = constrain(leftY, 0, maxY);
	rightX = constrain(rightX, 0, maxX);
	rightY = constrain(rightY, 0, maxY);

	_eyeLeft->eyeLxNext = leftX;
	_eyeLeft->eyeLyNext = leftY;
	_eyeRight->eyeLxNext = rightX;
	_eyeRight->eyeLyNext = rightY;

	_selectChannel(_left_channel);
	_eyeLeft->update();

	_selectChannel(_right_channel);
	_eyeRight->update();

	_lastMoveTime = millis();
}

void Eyes::setPosition(int x, int y)
{
	if (!_initialized)
		return;

	int maxX = _eyeLeft->getScreenConstraint_X();
	int maxY = _eyeLeft->getScreenConstraint_Y();

	x = constrain(x, 0, maxX);
	y = constrain(y, 0, maxY);

	_eyeLeft->eyeLxNext = x;
	_eyeLeft->eyeLyNext = y;
	_eyeRight->eyeLxNext = x;
	_eyeRight->eyeLyNext = y;

	_selectChannel(_left_channel);
	_eyeLeft->update();

	_selectChannel(_right_channel);
	_eyeRight->update();

	_lastMoveTime = millis();
}

void Eyes::setCyclopsMode(bool enable)
{
	if (!_initialized)
		return;

	_selectChannel(_left_channel);
	_eyeLeft->setCyclops(enable);

	_selectChannel(_right_channel);
	_eyeRight->setCyclops(enable);
}

// --- Приватные методы ---

void Eyes::_selectChannel(uint8_t channel)
{
	Wire.beginTransmission(_muxAddr);
	Wire.write(1 << channel);
	Wire.endTransmission();
	delayMicroseconds(50);
}

void Eyes::_updateAutoMove()
{
	unsigned long now = millis();
	if (now - _lastMoveTime >= MOVE_INTERVAL_MS)
	{
		_lastMoveTime = now;

		int maxX = _eyeLeft->getScreenConstraint_X();
		int maxY = _eyeLeft->getScreenConstraint_Y();

		int newX = random(maxX + 1);
		int newY = random(maxY + 1);

		_eyeLeft->eyeLxNext = newX;
		_eyeLeft->eyeLyNext = newY;
		_eyeRight->eyeLxNext = newX;
		_eyeRight->eyeLyNext = newY;

		_selectChannel(_left_channel);
		_eyeLeft->update();

		_selectChannel(_right_channel);
		_eyeRight->update();

		LOG_VERBOSE("Eyes", "Move to: (%d, %d)", newX, newY);
	}
}

void Eyes::_updateAutoBlink()
{
	unsigned long now = millis();
	if (now - _lastBlinkTime >= BLINK_INTERVAL_MS)
	{
		_lastBlinkTime = now;

		_selectChannel(_left_channel);
		_eyeLeft->blink();

		_selectChannel(_right_channel);
		_eyeRight->blink();

		LOG_VERBOSE("Eyes", "Blink");
	}
}

// Установка настроения по названию
void Eyes::setMood(const char *mood)
{
	if (!_initialized)
		return;

	if (strcmp(mood, "tired") == 0)
	{
		_currentMood = MOOD_TIRED;
	}
	else if (strcmp(mood, "angry") == 0)
	{
		_currentMood = MOOD_ANGRY;
	}
	else if (strcmp(mood, "happy") == 0)
	{
		_currentMood = MOOD_HAPPY;
	}
	else
	{
		_currentMood = MOOD_DEFAULT;
	}

	_applyMood();
}

// Установка настроения по индексу
void Eyes::setMood(uint8_t moodIndex)
{
	if (!_initialized)
		return;

	if (moodIndex >= MOOD_COUNT)
	{
		moodIndex = MOOD_DEFAULT;
	}

	_currentMood = moodIndex;
	_applyMood();
}

// Следующее настроение
void Eyes::nextMood()
{
	if (!_initialized)
		return;

	_currentMood++;
	if (_currentMood >= MOOD_COUNT)
	{
		_currentMood = 0;
	}

	_applyMood();
	LOG_DEBUG("Eyes", "Mood changed to: %s", getCurrentMoodName());
}

// Предыдущее настроение
void Eyes::previousMood()
{
	if (!_initialized)
		return;

	if (_currentMood == 0)
	{
		_currentMood = MOOD_COUNT - 1;
	}
	else
	{
		_currentMood--;
	}

	_applyMood();
	LOG_DEBUG("Eyes", "Mood changed to: %s", getCurrentMoodName());
}

// Получить название текущего настроения
const char *Eyes::getCurrentMoodName() const
{
	switch (_currentMood)
	{
	case MOOD_HAPPY:
		return "happy";
	case MOOD_ANGRY:
		return "angry";
	case MOOD_TIRED:
		return "tired";
	default:
		return "default";
	}
}

// Применить текущее настроение к глазам
void Eyes::_applyMood()
{
	// Используем значения напрямую из библиотеки
	uint8_t libraryMood = _currentMood;

	_selectChannel(_left_channel);
	_eyeLeft->setMood(libraryMood);

	_selectChannel(_right_channel);
	_eyeRight->setMood(libraryMood);

	LOG_VERBOSE("Eyes", "Applied mood: %s (code: %d)", getCurrentMoodName(), libraryMood);
}