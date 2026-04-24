#ifndef EYES_H
#define EYES_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>

// Не включаем FluxGarage_RoboEyes.h здесь!
// Вместо этого используем предварительное объявление

// Предварительное объявление шаблонного класса
template <typename T>
class RoboEyes;

class Eyes
{
public:
	// Конструктор: принимает адрес TCA9548A (по умолчанию 0x70)
	Eyes(uint8_t multiplexerAddr = 0x70, uint8_t left_channel = 0, uint8_t right_channel = 1);

	// Инициализация (вызывать в setup)
	bool begin();

	// Обновление состояния глаз (вызывать в loop)
	void update();

	// Принудительное моргание
	void blink();

	// Установка движения глаз (автоматическое или ручное)
	void setAutoMove(bool enable);
	void setAutoBlink(bool enable);

	// Ручное управление положением глаз (0-100 для X и Y)
	void setEyePosition(int leftX, int leftY, int rightX, int rightY);

	// Синхронная установка позиции для обоих глаз
	void setPosition(int x, int y);

	// Установка режима циклопа
	void setCyclopsMode(bool enable);

	// Установка настроения (эмоции) по названию
	void setMood(const char *mood);

	// Установка настроения по индексу (0-5)
	void setMood(uint8_t moodIndex);

	// Следующее настроение (циклический перебор)
	void nextMood();

	// Предыдущее настроение
	void previousMood();

	// Получить текущее настроение
	uint8_t getCurrentMood() const { return _currentMood; }

	// Получить название текущего настроения
	const char *getCurrentMoodName() const;

private:
	// TCA9548A
	uint8_t _muxAddr;
	uint8_t _left_channel;
	uint8_t _right_channel;

	// Дисплеи
	Adafruit_SH1106G _displayLeft;
	Adafruit_SH1106G _displayRight;

	// Глаза (используем указатели, так как тип не полностью определён)
	RoboEyes<Adafruit_SH1106G> *_eyeLeft;
	RoboEyes<Adafruit_SH1106G> *_eyeRight;

	// Состояние
	bool _initialized;
	bool _autoMove;
	bool _autoBlink;
	unsigned long _lastMoveTime;
	unsigned long _lastBlinkTime;

	// Текущая эмоция
	uint8_t _currentMood;

	// Константы
	static constexpr int MOVE_INTERVAL_MS = 3000;
	static constexpr int BLINK_INTERVAL_MS = 4000;

	// Доступные настроения
	static constexpr uint8_t MOOD_DEFAULT = 0;
	static constexpr uint8_t MOOD_TIRED = 1;
	static constexpr uint8_t MOOD_ANGRY = 2;
	static constexpr uint8_t MOOD_HAPPY = 3;
	static constexpr uint8_t MOOD_COUNT = 4;

	// Вспомогательные методы
	void _selectChannel(uint8_t channel);
	void _updateAutoMove();
	void _updateAutoBlink();
	void _applyMood();
};

#endif