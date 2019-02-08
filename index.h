const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
[type="date"] {
  background:#fff url('calendar_2.png')  97% 50% no-repeat ;
}
[type="date"]::-webkit-inner-spin-button {
  display: none;
}
[type="date"]::-webkit-calendar-picker-indicator {
  opacity: 0;
}
/*
[type="time"] {
  background:#fff url('calendar_2.png')  97% 50% no-repeat ;
}
[type="time"]::-webkit-inner-spin-button {
  display: none;
}
[type="time"]::-webkit-time-picker-indicator {
  opacity: 0;
}
*/
/* custom styles */
body {
  padding: 4em;
  background: #e5e5e5;
  font: 13px/1.4 Geneva, 'Lucida Sans', 'Lucida Grande', 'Lucida Sans Unicode', Verdana, sans-serif;
}
label {
  display: block;
}
input {
  border: 1px solid #c4c4c4;
  border-radius: 5px;
  background-color: #fff;
  padding: 3px 5px;
  box-shadow: inset 0 3px 6px rgba(0,0,0,0.1);
  width: 190px;
}
</style>
<body>

<h1>Alarm Clock</h1>
<label for="nowdate">Date</label><BR>
<input type="date" name="nowdate" id="nowdate"><BR>
<label for="nowtime">Time</label><BR>
<input type="time" name="nowtime" id="nowtime"><BR>
<label for="alarmtime">Alarm</label><BR>
<input type="time" name="alarmtime" id="alarmtime"><BR>
<button type="button" onclick="sendData(100)">Start Alarm Mode</button><BR>
<button type="button" onclick="setDate()">Set DateTime</button><BR>
<script>
function sendData(data) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setData?data=" + data, true);
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
