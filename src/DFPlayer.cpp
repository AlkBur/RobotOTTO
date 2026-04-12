#include "DFPlayer.h"
#include "Logger.h"

namespace
{
constexpr uint8_t kInitAttempts = 3;
constexpr unsigned long kInitialBootDelayMs = 800;
constexpr unsigned long kRetryBootDelayMs = 250;
constexpr unsigned long kRetryGapMs = 300;
constexpr unsigned long kCommandTimeoutMs = 1000;

void clearSerialInput(HardwareSerial &serial)
{
	while (serial.available() > 0)
	{
		serial.read();
		delay(0);
	}
}
} // namespace

DFPlayer::DFPlayer(uint8_t rxPin, uint8_t txPin)
	: _rxPin(rxPin), _txPin(txPin), _serial(nullptr), _initialized(false)
{
	_serial = &Serial1;
}

bool DFPlayer::begin(unsigned long baudRate)
{
	if (_serial == nullptr)
		return false;

	_initialized = false;
	_player.setTimeOut(kCommandTimeoutMs);

	for (uint8_t attempt = 1; attempt <= kInitAttempts; ++attempt)
	{
		const unsigned long bootDelay = (attempt == 1) ? kInitialBootDelayMs : kRetryBootDelayMs;

		_serial->end();
		delay(20);
		_serial->begin(baudRate, SERIAL_8N1, _rxPin, _txPin);
		delay(bootDelay);
		clearSerialInput(*_serial);

		LOG_INFO("DFPlayer", "Init attempt %u/%u", attempt, kInitAttempts);

		if (_player.begin(*_serial, true, true))
		{
			clearSerialInput(*_serial);
			_player.volume(15); // начальная громкость
			_initialized = true;
			LOG_INFO("DFPlayer", "Initialized successfully on attempt %u", attempt);
			return true;
		}

		const uint8_t type = _player.readType();
		const uint16_t value = _player.read();
		LOG_WARNING("DFPlayer", "Init attempt %u failed (type=%u, value=%u)", attempt, type, value);
		delay(kRetryGapMs);
	}

	LOG_ERROR("DFPlayer", "Initialization failed after %u attempts", kInitAttempts);
	return false;
}

void DFPlayer::play()
{
	if (!_initialized)
		return;
	_player.start();
}

void DFPlayer::stop()
{
	if (!_initialized)
		return;
	_player.stop();
}

void DFPlayer::next()
{
	if (!_initialized)
		return;
	_player.next();
}

void DFPlayer::previous()
{
	if (!_initialized)
		return;
	_player.previous();
}

void DFPlayer::pause()
{
	if (!_initialized)
		return;
	_player.pause();
}

void DFPlayer::setVolume(uint8_t volume)
{
	if (!_initialized)
		return;
	if (volume > 30)
		volume = 30;
	_player.volume(volume);
}

bool DFPlayer::isAvailable() const
{
	return _initialized;
}

bool DFPlayer::_isPlaying()
{
	if (!_initialized)
		return false;
	// Библиотека не предоставляет прямого метода для проверки,
	// но мы можем использовать функцию available() для получения статуса.
	// Более надежный способ — использовать пин BUSY, но для простоты используем этот.
	// Если плеер не занят, available() вернет false.
	return _player.available();
}

void DFPlayer::update()
{
	if (!_initialized)
		return;

	bool isPlayingNow = _isPlaying();
	if (isPlayingNow)
	{
		uint8_t type = _player.readType();
		Serial.print("DFPlayer type: ");
		Serial.println(type);
		if (type == 0x05)
		{
			_player.next();
			Serial.println("Track finished, playing next");
		}
	}
}

uint8_t DFPlayer::getStatus()
{
	if (!_initialized)
		return 99;

	return _player.readType();
}
