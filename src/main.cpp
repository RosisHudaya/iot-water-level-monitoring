#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define triggerPin D5
#define echoPin D6
int buzzerPin = D3;
int buzzerStatus = 0;
const int relayPin = D7;
int relayStatus = 0;

const char *ssid = "Galaxy A514696";
const char *password = "nyambung";
const char *mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

long now = millis();
long lastMeasure = 0;
String macAddr = "";

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  macAddr = WiFi.macAddress();
  Serial.println(macAddr);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(macAddr.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("");
  pinMode(relayPin, OUTPUT);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect(macAddr.c_str());
  }
  now = millis();
  if (now - lastMeasure > 5000)
  {
    lastMeasure = now;
    long duration, jarak;

    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);

    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH);

    jarak = duration * 0.034 / 2;

    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.home();
    lcd.print("Wtr-Lvl ");
    lcd.print(jarak);
    lcd.print("cm");

    if (jarak > 1)
    {
      Serial.print(jarak);
      Serial.println(" cm");
      Serial.println("Relay ON");

      buzzerStatus = 0;
      relayStatus = 1;

      digitalWrite(relayPin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Yey pompa nyala");
      digitalWrite(buzzerPin, LOW);
    }
    else
    {
      buzzerStatus = 0;
      relayStatus = 0;

      digitalWrite(buzzerPin, LOW);
      digitalWrite(relayPin, HIGH);
      Serial.print(jarak);
      Serial.println(" cm");
      Serial.println("Relay OFF");
      lcd.setCursor(0, 1);
      lcd.print("Yah pompa mati");
    }

    if (jarak < 5 && jarak > 1)
    {
      Serial.print(jarak);
      Serial.println(" cm");
      Serial.println("Relay ON");

      buzzerStatus = 1;
      relayStatus = 1;

      digitalWrite(relayPin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Yey pompa nyala");
      digitalWrite(buzzerPin, HIGH);
    }
  }
}