// This file contains the HTML data for the ESP32.

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=.8, maximum-scale=1, user-scalable=no">
  <style>
body {
  margin: 0;
  padding: 0;
  background-color: black;
  overflow: hidden;
  font-family: sans-serif;
}

h1 {
  margin: 10px 0 0 0;
  font-size: 1.8em;
  text-align: center;
  color: white;
}

input[type=range] {
  touch-action: none;
}

.noselect {
  -webkit-touch-callout: none;
  -webkit-user-select: none;
  user-select: none;
}

/*Throttle slider @ top center*/
.top-center {
  display: flex;
  justify-content: center;
  padding: 10px 10px 30px 10px;
}

.slidecontainer {
  width: 90%;
}

.slider {
  -webkit-appearance: none;
  appearance: none;
  width: 100%;
  height: 20px;
  border-radius: 5px;
  background: #d2d2d2;
  outline: none;
  opacity: 0.7;
  transition: opacity 0.2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 85px;
  height: 85px;
  border-radius: 50%;
  background: grey;
  border: none;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 60px;
  height: 40px;
  border-radius: 50%;
  background: white;
  cursor: pointer;
}

.bottom-controls {
  position: fixed;
  bottom: 120px;
  left: 60px;
}
.top-controls {
  position: fixed;
  bottom: 120px;
  right: -90px;
}

.aux-steering-group {
  display: flex;
  flex-direction: row;
  align-items: center;
  gap: 0px;
}

.aux-buttons {
  display: flex;
  flex-direction: column;
  gap: 85px;
  align-items: flex-start;
}

.auxButton {
  background-color: white;
  box-shadow: 5px 5px #888888;
  color: grey;
  padding: 30px 35px;
  border: none;
  border-radius: 20%;
  font-size: 24px;
  cursor: pointer;
  transform: rotate(90deg);
  transform-origin: left center;
  width: 150px;
}

.auxButton:active {
  transform: translate(-5px, 5px) rotate(90deg);
  transform-origin: left center;
  box-shadow: none;
}

.vertical-slider-container {
  height: 7vh;
  display: none;
  align-items: center;
  justify-content: center;
  transform:  translateX(-100px) translateY(-7vh);
  width: 80%;
}

.vertical-slider {
  writing-mode: bt-lr;
  width: 36vh;
  height: 20px;
  -webkit-appearance: none;
  appearance: none;
  background: #d2d2d2;
  outline: none;
  opacity: 0.7;
  transition: opacity 0.2s;
  transform: rotate(270deg);
}

.vertical-slider:hover {
  opacity: 1;
}

.vertical-slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  border-radius: 25%;
  width: 65px;
  height: 65px;
  background-color: grey;
  border: none;
  cursor: pointer;
}

.vertical-slider::-moz-range-thumb {
  width: 60px;
  height: 40px;
  border-radius: 50%;
  background: white;
  cursor: pointer;
}
</style>
</head>

<body class="noselect">
  <h1>TableRacer_1</h1> <!-- customize Name here too -->
	<br/>
	<br/>
  <div class="top-center">
    <div class="slidecontainer">
      <input type="range" min="40" max="132" value="86" class="slider" id="throttle"
         ontouchend='resetThrottleSlider()'>
    </div>
  </div>

  <div class="bottom-controls">
    <div class="aux-steering-group">
      <div class="aux-buttons">
        <button class="auxButton"
          ontouchstart='sendButtonInput("steering", "132")'
          onmousedown='sendButtonInput("steering", "132")'
          ontouchend='sendButtonInput("steering", "86")'
          onmouseup='sendButtonInput("steering", "86")'>&#8678;LEFT</button>

        <button class="auxButton"
          ontouchstart='sendButtonInput("steering", "40")'
          onmousedown='sendButtonInput("steering", "40")'
          ontouchend='sendButtonInput("steering", "86")'
          onmouseup='sendButtonInput("steering", "86")'>RIGHT&#8680;</button>        
      </div>

      <div class="vertical-slider-container">
        <input type="range" min="40" max="132" value="86" class="vertical-slider" id="steering"
           ontouchend='resetSteeringSlider()' >
      </div>
    </div>
  </div>
  <div class="top-controls">
	<div class="aux-buttons">
		<button class="auxButton" id="toggleControlMode" onclick="toggleControlMode()">Toggle UI</button>
		<button class="auxButton"
          ontouchstart='sendButtonInput("coupling", "")'
          onmousedown='sendButtonInput("coupling", "")'>COUPLE</button>
	</div>
  </div>
</body>
  
<script>
  // ======================================================================================================
  // Select wheter you want the Slider or Buttons as default steering controls (true=Slider, false=Buttons)
  var useSlider = false;
  // ======================================================================================================
  var steeringValue = 86;
  var throttleValue = 86;

  var webSocketCarInputUrl = "ws://" + window.location.hostname + "/CarInput";
  var websocketCarInput;

  var toggleControlMode = function() {
    useSlider = !useSlider;

    var slider = document.querySelector('.vertical-slider-container');
    var buttons = document.querySelector('.aux-buttons');
    var allButtons = buttons.querySelectorAll('button');
    var steeringSlider = document.getElementById('steering');

    if (useSlider) {
      slider.style.display = 'flex';
      buttons.style.display = 'none';
      for (var i = 0; i < allButtons.length; i++) {
        allButtons[i].disabled = true;
      }
      steeringSlider.disabled = false;
      document.getElementById('toggleControlMode').innerText = 'Buttons';
    } else {
      slider.style.display = 'none';
      buttons.style.display = 'flex';
      for (var i = 0; i < allButtons.length; i++) {
        allButtons[i].disabled = false;
      }
      steeringSlider.disabled = true;
      document.getElementById('toggleControlMode').innerText = 'Slider';
    }
  };

  var initCarInputWebSocket = function() {
    websocketCarInput = new WebSocket(webSocketCarInputUrl);
    websocketCarInput.onclose = function() {
      setTimeout(initCarInputWebSocket, 2000);
    };
  };

  var sendButtonInput = function(key, value) {
    if (websocketCarInput && websocketCarInput.readyState === 1) {
      websocketCarInput.send(key + "," + value);
    }
  };

  document.getElementById('throttle').oninput = function(e) {
    throttleValue = e.target.value;
  };

  document.getElementById('steering').oninput = function(e) {
    steeringValue = e.target.value;
  };

  var resetThrottleSlider = function() {
    throttleValue = 86;
    document.getElementById('throttle').value = 86;
  };

  var resetSteeringSlider = function() {
    steeringValue = 86;
    document.getElementById('steering').value = 86;
  };

  setInterval(function() {
    if (websocketCarInput && websocketCarInput.readyState === 1) {
      websocketCarInput.send("throttle," + throttleValue);
      if (useSlider) {
        websocketCarInput.send("steering," + steeringValue);
      }
    }
  }, 20); // 50 Hz

  window.onload = function() {
      // start WebSocket
      initCarInputWebSocket();

      // initily show controlls
      if (useSlider) {
          document.querySelector('.vertical-slider-container').style.display = 'flex';
          document.querySelector('.aux-buttons').style.display = 'none';
          document.getElementById('steering').disabled = false;
          document.getElementById('toggleControlMode').innerText = 'Buttons';
      } else {
          document.querySelector('.vertical-slider-container').style.display = 'none';
          document.querySelector('.aux-buttons').style.display = 'flex';
          document.getElementById('steering').disabled = true;
          document.getElementById('toggleControlMode').innerText = 'Slider';
      }
  };

</script>

  </body>    
</html>
)HTMLHOMEPAGE";