#include <WiFiS3.h>

char ssid[] = "Airtel_reha_2398";
char pass[] = "air72751";

WiFiServer server(80);

const int LED_PIN = 8;
const int SWITCH_PIN = 7;

bool lightState = false;
bool lastSwitchState = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(5000);
    Serial.println("Connecting...");
  }

  Serial.println("Wi-Fi connected!");
  Serial.print("Arduino IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {

  // Physical switch
  bool currentSwitchState = digitalRead(SWITCH_PIN);

  if (currentSwitchState != lastSwitchState) {
    delay(30);
    currentSwitchState = digitalRead(SWITCH_PIN);

    if (currentSwitchState != lastSwitchState) {
      lightState = (currentSwitchState == LOW);
      lastSwitchState = currentSwitchState;
    }
  }

  // Phone
  WiFiClient client = server.available();

  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("GET /on") >= 0)
      lightState = true;

    if (request.indexOf("GET /off") >= 0)
      lightState = false;

    // Live status
    if (request.indexOf("GET /status") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();

      client.println(lightState ? "ON" : "OFF");

      client.stop();
      digitalWrite(LED_PIN, lightState ? HIGH : LOW);
      return;
    }

    // Dashboard
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.println("<!DOCTYPE html>");
    client.println("<html><head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");

    client.println("<style>");
    client.println("body{font-family:Arial;text-align:center;background:#000;color:white;margin:0;padding:25px}");
    client.println("h1{margin:10px 0 5px}");
    client.println(".card{max-width:400px;margin:30px auto;padding:25px;background:#222;border-radius:15px}");
    client.println("button{padding:12px 30px;margin:8px;font-size:18px;border:0;border-radius:8px}");
    client.println(".status{margin-top:20px;font-size:18px}");
    client.println("</style>");

    client.println("<script>");
    client.println("function update(){fetch('/status').then(r=>r.text()).then(s=>document.getElementById('state').innerText=s)}");
    client.println("setInterval(update,1000);");
    client.println("</script>");

    client.println("</head><body>");

    client.println("<h1>SMART HOME</h1>");
    client.println("<div>CONTROL PANEL</div>");

    client.println("<div class='card'>");
    client.println("<h2>LIGHT</h2>");

    client.println("<a href='/on'><button>ON</button></a>");
    client.println("<a href='/off'><button>OFF</button></a>");

    client.println("<div class='status'>STATUS: <b id='state'>");
    client.println(lightState ? "ON" : "OFF");
    client.println("</b></div>");

    client.println("</div>");
    client.println("</body></html>");

    client.stop();
  }

  digitalWrite(LED_PIN, lightState ? HIGH : LOW);
}