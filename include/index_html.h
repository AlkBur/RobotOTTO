#pragma once
#include <Arduino.h>

// WiFi
const char *WIFI_SSID = "ESP32-Robot";
const char *WIFI_PASS = "12345678";

const char htmlContent[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no, viewport-fit=cover">
	<title>ESP32 Robot Controller</title>
	<style>
		* {
			user-select: none;
			touch-action: manipulation;
			box-sizing: border-box;
		}

		body {
			background: #1a1e2a;
			font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
			margin: 0;
			padding: 16px;
			min-height: 100vh;
			display: flex;
			justify-content: center;
			/* центрирование по горизонтали */
			align-items: flex-start;
			/* прижимаем к верху по вертикали */
		}

		.container {
			max-width: 600px;
			width: 100%;
			background: #0f1222;
			border-radius: 48px;
			padding: 20px;
			box-shadow: 0 20px 35px rgba(0, 0, 0, 0.5);
			/* margin: 0; - не задаём, оставляем как есть */
		}

		.status-bar {
			background: #2a2f3f;
			border-radius: 40px;
			padding: 8px 20px;
			margin-bottom: 24px;
			text-align: center;
			font-weight: bold;
			color: #eee;
			display: flex;
			justify-content: space-between;
			flex-wrap: wrap;
		}

		.status-led {
			display: inline-block;
			width: 12px;
			height: 12px;
			border-radius: 50%;
			background: #ff5555;
			margin-right: 8px;
			transition: background 0.2s;
		}

		.status-led.connected {
			background: #55ff55;
			box-shadow: 0 0 8px #55ff55;
		}

		/* Tabs */
		.tabs {
			display: flex;
			gap: 8px;
			margin-bottom: 24px;
			background: #202433;
			padding: 6px;
			border-radius: 60px;
		}

		.tab-btn {
			flex: 1;
			background: transparent;
			border: none;
			color: #aaa;
			font-size: 1rem;
			font-weight: bold;
			padding: 10px 0;
			border-radius: 40px;
			transition: 0.2s;
			cursor: pointer;
		}

		.tab-btn.active {
			background: #4c5a7a;
			color: white;
			box-shadow: 0 2px 6px rgba(0, 0, 0, 0.3);
		}

		.tab-pane {
			display: none;
			animation: fade 0.2s ease;
			margin-top: 0;
			/* прижимаем к верху */
		}

		.tab-pane.active {
			display: block;
		}

		@keyframes fade {
			from {
				opacity: 0;
			}

			to {
				opacity: 1;
			}
		}

		/* Joystick */
		.joystick-area {
			background: #202433;
			border-radius: 50%;
			width: 100%;
			aspect-ratio: 1 / 1;
			margin-bottom: 32px;
			position: relative;
			display: flex;
			justify-content: center;
			align-items: center;
			box-shadow: inset 0 0 12px rgba(0, 0, 0, 0.6), 0 8px 18px rgba(0, 0, 0, 0.3);
		}

		canvas {
			width: 100%;
			height: 100%;
			display: block;
			border-radius: 50%;
			touch-action: none;
		}

		.dpad-row {
			display: flex;
			justify-content: center;
			gap: 30px;
			margin-top: 10px;
		}

		.dpad-btn {
			background: #2e3440;
			width: 80px;
			height: 80px;
			border-radius: 24px;
			display: flex;
			align-items: center;
			justify-content: center;
			font-size: 2.4rem;
			font-weight: bold;
			color: white;
			box-shadow: 0 8px 0 #0b0e16;
			transition: all 0.05s linear;
			cursor: pointer;
		}

		.dpad-btn:active,
		.dpad-btn.active {
			transform: translateY(4px);
			box-shadow: 0 2px 0 #0b0e16;
			background: #4c5a7a;
		}

		/* Sliders and buttons */
		.slider-container {
			background: #202433;
			border-radius: 32px;
			padding: 20px;
			margin-bottom: 24px;
		}

		.slider-label {
			display: flex;
			justify-content: space-between;
			color: white;
			margin-bottom: 12px;
			font-weight: bold;
		}

		input[type="range"] {
			width: 100%;
			height: 8px;
			-webkit-appearance: none;
			background: #3a4055;
			border-radius: 5px;
			outline: none;
		}

		input[type="range"]:focus {
			outline: none;
		}

		input[type="range"]::-webkit-slider-thumb {
			-webkit-appearance: none;
			width: 28px;
			height: 28px;
			border-radius: 50%;
			background: #ffaa44;
			cursor: pointer;
			box-shadow: 0 2px 8px black;
		}

		.button-group {
			display: flex;
			gap: 16px;
			justify-content: center;
			margin-top: 20px;
			flex-wrap: wrap;
		}

		.ctrl-btn {
			background: #2e3440;
			border: none;
			color: white;
			font-size: 1.2rem;
			font-weight: bold;
			padding: 12px 24px;
			border-radius: 40px;
			box-shadow: 0 4px 0 #0b0e16;
			transition: 0.05s linear;
			cursor: pointer;
			min-width: 110px;
		}

		.ctrl-btn:active {
			transform: translateY(2px);
			box-shadow: 0 1px 0 #0b0e16;
		}

		.icon-btn {
			font-size: 2rem;
			padding: 12px;
			min-width: 70px;
		}

		.value-display {
			text-align: center;
			margin-top: 16px;
			color: #bbffcc;
			font-family: monospace;
			font-size: 1.1rem;
		}

		@media (max-width: 480px) {
			.dpad-btn {
				width: 70px;
				height: 70px;
				font-size: 2rem;
			}

			.ctrl-btn {
				padding: 10px 18px;
				font-size: 1rem;
				min-width: 90px;
			}
		}
	</style>
</head>

<body>
	<div class="container">
		<div class="status-bar">
			<span>🤖 ESP32 Robot</span>
			<span><span id="wsLed" class="status-led"></span> <span id="wsStatus">Отключено</span></span>
		</div>

		<div class="tabs">
			<button class="tab-btn active" data-tab="tab1">🎮 Управление</button>
			<button class="tab-btn" data-tab="tab2">🔊 Медиа</button>
			<button class="tab-btn" data-tab="tab3">🤸 Движения</button>
			<button class="tab-btn" data-tab="tab4">🦵 Ноги</button>
		</div>

		<!-- Вкладка 1: Джойстик + D-Pad (Up/Down) -->
		<div id="tab1" class="tab-pane active">
			<div class="joystick-area">
				<canvas id="joystickCanvas" width="500" height="500"></canvas>
			</div>
			<div class="dpad-row">
				<div class="dpad-btn" data-dir="U">▲</div>
				<div class="dpad-btn" data-dir="D">▼</div>
			</div>
			<div class="value-display">
				🕹️ Джойстик: <span id="joyVal">X:0 Y:0</span> &nbsp;|&nbsp; D-Pad: <span id="dpadVal">—</span>
			</div>
		</div>

		<!-- Вкладка 2: Громкость + кнопки плеера -->
		<div id="tab2" class="tab-pane">
			<div class="slider-container">
				<div class="slider-label">
					<span>🔊 Громкость</span>
					<span id="volValue">15</span>
				</div>
				<input type="range" id="volumeSlider" min="0" max="30" value="15" step="1">
			</div>
			<div class="button-group">
				<button id="playPauseBtn" class="ctrl-btn">⏸️ Пауза</button>
				<button id="prevBtn" class="ctrl-btn">⏮️ Пред.</button>
				<button id="nextBtn" class="ctrl-btn">⏭️ След.</button>
			</div>
		</div>

		<!-- Вкладка 3: Предопределённые движения (иконки) -->
		<div id="tab3" class="tab-pane">
			<div class="button-group">
				<button id="moveCross" class="ctrl-btn icon-btn">❌</button>
				<button id="moveCircle" class="ctrl-btn icon-btn">⭕</button>
				<button id="moveSquare" class="ctrl-btn icon-btn">⬛</button>
				<button id="moveTriangle" class="ctrl-btn icon-btn">▲</button>
			</div>
			<div class="value-display" style="margin-top: 30px;">
				Нажмите на иконку для отправки движения
			</div>
		</div>

		<!-- Вкладка 4: Два ползунка для ног (0-180) -->
		<div id="tab4" class="tab-pane">
			<div class="slider-container">
				<div class="slider-label">
					<span>🦵 Нога левая (угол)</span>
					<span id="leg1Value">90</span>
				</div>
				<input type="range" id="leg1Slider" min="0" max="180" value="90" step="1">
			</div>
			<div class="slider-container">
				<div class="slider-label">
					<span>🦵 Нога правая (угол)</span>
					<span id="leg2Value">90</span>
				</div>
				<input type="range" id="leg2Slider" min="0" max="180" value="90" step="1">
			</div>
		</div>
	</div>

	<script>
		// --- WebSocket ---
		let ws = null;
		const wsLed = document.getElementById('wsLed');
		const wsStatusSpan = document.getElementById('wsStatus');

		function connectWebSocket() {
			const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
			const host = window.location.hostname;
			const port = window.location.port || '80';
			const url = `${protocol}//${host}:${port}/ws`;
			ws = new WebSocket(url);
			ws.onopen = () => {
				wsLed.classList.add('connected');
				wsStatusSpan.innerText = 'Подключено';
			};
			ws.onclose = () => {
				wsLed.classList.remove('connected');
				wsStatusSpan.innerText = 'Отключено';
				setTimeout(connectWebSocket, 3000);
			};
			ws.onerror = (err) => ws.close();
		}

		function sendCompact(cmd) {
			if (ws && ws.readyState === WebSocket.OPEN) {
				ws.send(cmd);
				console.log("Send:", cmd);
			} else {
				console.warn("WebSocket not ready");
			}
		}

		// --- Джойстик (canvas) ---
		const canvas = document.getElementById('joystickCanvas');
		const ctx = canvas.getContext('2d');
		let joystickActive = false;
		let joyX = 0, joyY = 0;
		let joyCenterX = 0, joyCenterY = 0;
		let joyRadius = 0;
		let stickX = 0, stickY = 0;

		function resizeCanvas() {
			const rect = canvas.getBoundingClientRect();
			canvas.width = rect.width;
			canvas.height = rect.height;
			joyCenterX = canvas.width / 2;
			joyCenterY = canvas.height / 2;
			joyRadius = Math.min(canvas.width, canvas.height) * 0.4;
			if (!joystickActive) {
				stickX = joyCenterX;
				stickY = joyCenterY;
				joyX = 0; joyY = 0;
			}
			drawJoystick();
		}

		function drawJoystick() {
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			ctx.beginPath();
			ctx.arc(joyCenterX, joyCenterY, joyRadius, 0, 2 * Math.PI);
			ctx.fillStyle = '#2c3042';
			ctx.fill();
			ctx.strokeStyle = '#5f6a8a';
			ctx.lineWidth = 3;
			ctx.stroke();
			ctx.beginPath();
			ctx.arc(stickX, stickY, joyRadius * 0.4, 0, 2 * Math.PI);
			ctx.fillStyle = '#ffaa44';
			ctx.fill();
			ctx.strokeStyle = '#cc8822';
			ctx.lineWidth = 2;
			ctx.stroke();
		}

		function formatSigned3(value) {
			let intVal = Math.round(value * 100);
			intVal = Math.min(100, Math.max(-100, intVal));
			const sign = intVal >= 0 ? '+' : '-';
			let absVal = Math.abs(intVal);
			let numStr = absVal.toString().padStart(3, '0');
			return sign + numStr;
		}

		function sendJoystick() {
			let xStr = formatSigned3(joyX);
			let yStr = formatSigned3(joyY);
			sendCompact('J' + xStr + yStr);
			document.getElementById('joyVal').innerHTML = `X:${(joyX * 100).toFixed(0)} Y:${(joyY * 100).toFixed(0)}`;
		}

		function updateJoystickPosition(clientX, clientY) {
			const rect = canvas.getBoundingClientRect();
			let rawX = clientX - rect.left;
			let rawY = clientY - rect.top;
			let dx = rawX - joyCenterX;
			let dy = rawY - joyCenterY;
			const dist = Math.hypot(dx, dy);
			if (dist > joyRadius) {
				dx = dx * (joyRadius / dist);
				dy = dy * (joyRadius / dist);
			}
			stickX = joyCenterX + dx;
			stickY = joyCenterY + dy;
			joyX = dx / joyRadius;
			joyY = dy / joyRadius;
			joyX = Math.min(1, Math.max(-1, joyX));
			joyY = Math.min(1, Math.max(-1, joyY));

			// ✅ Инвертируем Y, чтобы вверх давало положительное значение
			joyY = -joyY;

			drawJoystick();
			sendJoystick();
		}

		function resetJoystick() {
			joystickActive = false;
			stickX = joyCenterX;
			stickY = joyCenterY;
			joyX = 0; joyY = 0;
			drawJoystick();
			sendJoystick();
		}

		function onStart(e) {
			e.preventDefault();
			joystickActive = true;
			const point = e.touches ? e.touches[0] : e;
			updateJoystickPosition(point.clientX, point.clientY);
		}
		function onMove(e) {
			if (!joystickActive) return;
			e.preventDefault();
			const point = e.touches ? e.touches[0] : e;
			updateJoystickPosition(point.clientX, point.clientY);
		}
		function onEnd(e) {
			if (!joystickActive) return;
			e.preventDefault();
			resetJoystick();
		}
		canvas.addEventListener('touchstart', onStart);
		canvas.addEventListener('touchmove', onMove);
		canvas.addEventListener('touchend', onEnd);
		canvas.addEventListener('mousedown', onStart);
		window.addEventListener('mousemove', onMove);
		window.addEventListener('mouseup', onEnd);
		window.addEventListener('resize', () => resizeCanvas());
		resizeCanvas();

		// --- D-Pad только Up/Down ---
		const dpadBtns = document.querySelectorAll('.dpad-btn');
		let activeDpad = null;
		function sendDpad(dir, state) {
			let cmd = 'D' + dir + (state ? '1' : '0');
			sendCompact(cmd);
			const span = document.getElementById('dpadVal');
			if (state === 1) span.innerText = dir === 'U' ? 'Вверх' : 'Вниз';
			else if (activeDpad === null) span.innerText = '—';
		}
		function handleDpadStart(dir) {
			if (activeDpad === dir) return;
			if (activeDpad !== null) sendDpad(activeDpad, 0);
			activeDpad = dir;
			sendDpad(dir, 1);
		}
		function handleDpadEnd(dir) {
			if (activeDpad !== dir) return;
			sendDpad(dir, 0);
			activeDpad = null;
		}
		dpadBtns.forEach(btn => {
			const dir = btn.getAttribute('data-dir');
			btn.addEventListener('touchstart', (e) => {
				e.preventDefault();
				handleDpadStart(dir);
				btn.classList.add('active');
			});
			btn.addEventListener('touchend', (e) => {
				e.preventDefault();
				handleDpadEnd(dir);
				btn.classList.remove('active');
			});
			btn.addEventListener('mousedown', (e) => {
				e.preventDefault();
				handleDpadStart(dir);
				btn.classList.add('active');
			});
			window.addEventListener('mouseup', () => {
				if (activeDpad === dir) {
					handleDpadEnd(dir);
					btn.classList.remove('active');
				}
			});
			btn.addEventListener('mouseleave', () => {
				if (activeDpad === dir) {
					handleDpadEnd(dir);
					btn.classList.remove('active');
				}
			});
		});

		// --- Вкладка 2: громкость и плеер ---
		const volumeSlider = document.getElementById('volumeSlider');
		const volValue = document.getElementById('volValue');
		function sendVolume(val) {
			let cmd = 'V' + val.toString(); // например "V15"
			sendCompact(cmd);
			volValue.innerText = val;
		}
		volumeSlider.addEventListener('input', (e) => {
			let val = e.target.value;
			volValue.innerText = val;
			sendVolume(val);
		});
		// Кнопки плеера
		let playState = false; // true = воспроизведение, false = пауза
		const playPauseBtn = document.getElementById('playPauseBtn');
		function sendPlayPause() {
			playState = !playState;
			let cmd = playState ? "PP0" : "PP1";   // PP0 = воспроизведение, PP1 = пауза (или наоборот)
			// Можно использовать "PLAY" / "PAUS". Для простоты: "PP" + state
			sendCompact(cmd);
			playPauseBtn.innerHTML = playState ? "⏸️ Пауза" : "▶️ Пуск";
		}
		playPauseBtn.addEventListener('click', sendPlayPause);
		document.getElementById('prevBtn').addEventListener('click', () => sendCompact("PREV"));
		document.getElementById('nextBtn').addEventListener('click', () => sendCompact("NEXT"));

		// --- Вкладка 3: предопределённые движения ---
		document.getElementById('moveCross').addEventListener('click', () => sendCompact("M0"));   // крест
		document.getElementById('moveCircle').addEventListener('click', () => sendCompact("M1"));  // круг
		document.getElementById('moveSquare').addEventListener('click', () => sendCompact("M2"));  // квадрат
		document.getElementById('moveTriangle').addEventListener('click', () => sendCompact("M3")); // треугольник

		// --- Вкладка 4: ползунки ног (0-180) ---
		const leg1Slider = document.getElementById('leg1Slider');
		const leg1Value = document.getElementById('leg1Value');
		const leg2Slider = document.getElementById('leg2Slider');
		const leg2Value = document.getElementById('leg2Value');

		function sendLeg(leg, angle) {
			let angleStr = angle.toString().padStart(3, '0'); // "000" - "180"
			sendCompact(`L${leg}${angleStr}`);
		}
		leg1Slider.addEventListener('input', (e) => {
			let val = e.target.value;
			leg1Value.innerText = val;
			sendLeg(0, val);
		});
		leg2Slider.addEventListener('input', (e) => {
			let val = e.target.value;
			leg2Value.innerText = val;
			sendLeg(1, val);
		});
		// начальная отправка значений по умолчанию
		sendLeg(0, 90);
		sendLeg(1, 90);
		sendVolume(15);

		// --- Вкладки (переключение) ---
		const tabs = document.querySelectorAll('.tab-btn');
		const panes = document.querySelectorAll('.tab-pane');
		tabs.forEach(btn => {
			btn.addEventListener('click', () => {
				const tabId = btn.getAttribute('data-tab');
				tabs.forEach(b => b.classList.remove('active'));
				panes.forEach(p => p.classList.remove('active'));
				btn.classList.add('active');
				document.getElementById(tabId).classList.add('active');
				// при переключении вкладки можно сбросить состояние D-Pad, чтобы не висло нажатие
				if (activeDpad) {
					sendDpad(activeDpad, 0);
					activeDpad = null;
				}
				// также сброс джойстика при уходе с вкладки
				if (joystickActive) {
					resetJoystick();
				}
			});
		});

		connectWebSocket();
		// пинг каждые 25 сек
		setInterval(() => {
			if (ws && ws.readyState === WebSocket.OPEN) sendCompact("P");
		}, 25000);
	</script>
</body>

</html>
)rawliteral";