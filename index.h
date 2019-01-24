const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<div id="demo">
<h1>The ESP8266 Clock Test</h1>
	<button type="button" onclick="sendData(1)">LED ON</button>
	<button type="button" onclick="sendData(0)">LED OFF</button>
  <button type="button" onclick="setDate()">Set DateTime</button><BR>
</div>

<div>
	SYS Date is : <span id="SYSDate">NA</span><br>
  ESP Date is : <span id="ESPDate">NA</span><br>
  LED State is : <span id="LEDState">NA</span>
</div>
<script>
function sendData(led) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setLED?LEDstate="+led, true);
  xhttp.send();
}

function setDate() {
  var date = new Date();
  
  var s = "date=" + date.toString();
  s += "&year=" + date.getFullYear();
  s += "&month=" + parseInt(date.getMonth() + 1);
  s += "&day=" + date.getDate();
  s += "&hours=" + date.getHours();
  s += "&minutes=" + date.getMinutes();
  s += "&seconds=" + date.getSeconds();
  
  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "setDate", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send(s); 
}

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getDate();
}, 2000); //2000mSeconds update rate

function getDate() {  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var date = new Date();
      document.getElementById("SYSDate").innerHTML = date.toString();
      document.getElementById("ESPDate").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "getDate", true);
  xhttp.send();
}
</script>
</body>
</html>

)=====";
