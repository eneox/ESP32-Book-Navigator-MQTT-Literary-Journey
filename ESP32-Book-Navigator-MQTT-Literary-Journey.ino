#include <GxEPD.h>
#include <GxDEPG0266BN/GxDEPG0266BN.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <PubSubClient.h>
#include <WiFi.h>

GxIO_Class io(SPI, /*CS=5*/ 17, /*DC=*/ 16, /*RST=*/ 15);
GxEPD_Class display(io, /*RST=*/ 15, /*BUSY=*/ 14);

const char books[][300] = {
  "Book 1 Text Here...",
  "Book 2 Text Here...",
  "Book 3 Text Here..."
};

int currentPage = 0;
int totalBooks = sizeof(books) / sizeof(books[0]);

const char ssid[] = "your_SSID";
const char password[] = "your_PASSWORD";
const char mqtt_server[] = "3.77.175.147";
const char mqtt_topic[] = "bookControl";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  connectWiFi();
  connectMQTT();
  display.init(115200);
  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  drawBookText(books[currentPage]);
  display.update();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi");
}

void connectMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.subscribe(mqtt_topic);
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void drawBookText(const char* text) {
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;

  display.setCursor(x, y);
  display.print(text);
}

void nextPage() {
  currentPage = (currentPage + 1) % totalBooks;
  display.fillScreen(GxEPD_WHITE);
  drawBookText(books[currentPage]);
  display.update();
}

void prevPage() {
  currentPage = (currentPage - 1 + totalBooks) % totalBooks;
  display.fillScreen(GxEPD_WHITE);
  drawBookText(books[currentPage]);
  display.update();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived in topic: " + String(topic));

  if (String(topic) == mqtt_topic) {
    char controlCommand = (char)payload[0];

    if (controlCommand == '1') {
      nextPage();
    } else if (controlCommand == '2') {
      prevPage();
    }
  }
}
