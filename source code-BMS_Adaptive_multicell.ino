/************************************************************
 * ADAPTIVE MULTI-CELL BATTERY INTELLIGENCE ENGINE
 * ESP32 + Velxio + Blynk
 *
 * Blynk Datastreams:
 * V0 = Cell 1 Voltage
 * V1 = Cell 2 Voltage
 * V2 = Cell 3 Voltage
 * V3 = Cell 4 Voltage
 * V4 = Average Voltage
 * V5 = Imbalance %
 * V6 = Battery Status
 * V7 = Protection Status
 * V8 = Battery Health Code
 * V9 = Protection Status Code
 ************************************************************/

#define BLYNK_TEMPLATE_ID "TMPL30J3K6Yhz"
#define BLYNK_TEMPLATE_NAME "smart BMS"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN_HERE"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ==========================================================
// WIFI CREDENTIALS
// ==========================================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ==========================================================
// BMS ANALOG INPUTS
// ==========================================================

const int CELL1_PIN = 34;
const int CELL2_PIN = 35;
const int CELL3_PIN = 32;
const int CELL4_PIN = 33;

// ==========================================================
// SIMULATION VOLTAGE RANGE
// ==========================================================

const float SIM_MIN_VOLTAGE = 2.5;
const float SIM_MAX_VOLTAGE = 4.3;

// ==========================================================
// BMS THRESHOLDS
// ==========================================================

const float MIN_CELL_VOLTAGE = 3.0;
const float MAX_CELL_VOLTAGE = 4.2;

const float MINOR_IMBALANCE = 0.10;
const float CRITICAL_IMBALANCE = 0.30;

const float SENSOR_MIN_VALID = 0.5;

// ==========================================================
// TIMING
// ==========================================================

const unsigned long SENSOR_INTERVAL = 100;
const unsigned long SERIAL_INTERVAL = 1000;
const unsigned long WIFI_CHECK_INTERVAL = 5000;

// ==========================================================
// BATTERY VARIABLES
// ==========================================================

float cellVoltage[4] = {0, 0, 0, 0};

float averageVoltage = 0;
float minimumVoltage = 0;
float maximumVoltage = 0;
float imbalanceVoltage = 0;
float imbalancePercent = 0;

int weakestCell = 0;
int strongestCell = 0;

// ==========================================================
// STATUS
// ==========================================================

String batteryHealth = "STARTING";
String protectionStatus = "NORMAL";

int batteryHealthCode = 0;
int protectionStatusCode = 0;

// ==========================================================
// EVENT-DRIVEN CLOUD TELEMETRY
// ==========================================================

float previousCellVoltage[4] =
{
  -100.0, -100.0, -100.0, -100.0
};

float previousAverageVoltage = -100.0;
float previousImbalancePercent = -100.0;

String previousBatteryHealth = "";
String previousProtectionStatus = "";

const float CLOUD_VOLTAGE_CHANGE = 0.02;
const float CLOUD_IMBALANCE_CHANGE = 0.02;

bool cloudSyncPending = true;

// ==========================================================
// CONNECTION TRACKING
// ==========================================================

bool wasBlynkConnected = false;

unsigned long lastSensorRead = 0;
unsigned long lastSerialPrint = 0;
unsigned long lastWiFiCheck = 0;

// ==========================================================
// FAULT HISTORY
// ==========================================================

String faultHistory[10];
int faultCount = 0;

String previousLoggedHealth = "";

// ==========================================================
// READ CELL VOLTAGES
// ==========================================================

float readSimulatedVoltage(int pin)
{
  int rawValue = analogRead(pin);

  float voltage =
    SIM_MIN_VOLTAGE +
    ((float)rawValue / 4095.0) *
    (SIM_MAX_VOLTAGE - SIM_MIN_VOLTAGE);

  return voltage;
}

void readCellVoltages()
{
  cellVoltage[0] = readSimulatedVoltage(CELL1_PIN);
  cellVoltage[1] = readSimulatedVoltage(CELL2_PIN);
  cellVoltage[2] = readSimulatedVoltage(CELL3_PIN);
  cellVoltage[3] = readSimulatedVoltage(CELL4_PIN);
}

// ==========================================================
// CALCULATE BATTERY PARAMETERS
// ==========================================================

void calculateBatteryParameters()
{
  minimumVoltage = cellVoltage[0];
  maximumVoltage = cellVoltage[0];

  weakestCell = 0;
  strongestCell = 0;

  float total = 0;

  for (int i = 0; i < 4; i++)
  {
    total += cellVoltage[i];

    if (cellVoltage[i] < minimumVoltage)
    {
      minimumVoltage = cellVoltage[i];
      weakestCell = i;
    }

    if (cellVoltage[i] > maximumVoltage)
    {
      maximumVoltage = cellVoltage[i];
      strongestCell = i;
    }
  }

  averageVoltage = total / 4.0;

  imbalanceVoltage = maximumVoltage - minimumVoltage;

  if (averageVoltage > 0)
  {
    imbalancePercent =
      (imbalanceVoltage / averageVoltage) * 100.0;
  }
  else
  {
    imbalancePercent = 100.0;
  }
}

// ==========================================================
// SENSOR VALIDITY
// ==========================================================

bool sensorFaultDetected()
{
  for (int i = 0; i < 4; i++)
  {
    if (cellVoltage[i] < SENSOR_MIN_VALID)
    {
      return true;
    }

    if (isnan(cellVoltage[i]))
    {
      return true;
    }
  }

  return false;
}

// ==========================================================
// BATTERY HEALTH
// ==========================================================

void determineBatteryHealth()
{
  if (sensorFaultDetected())
  {
    batteryHealth = "PACK FAILURE";
    batteryHealthCode = 3;
    return;
  }

  for (int i = 0; i < 4; i++)
  {
    if (cellVoltage[i] < MIN_CELL_VOLTAGE ||
        cellVoltage[i] > MAX_CELL_VOLTAGE)
    {
      batteryHealth = "PACK FAILURE";
      batteryHealthCode = 3;
      return;
    }
  }

  if (imbalanceVoltage >= CRITICAL_IMBALANCE)
  {
    batteryHealth = "CRITICAL";
    batteryHealthCode = 2;
  }
  else if (imbalanceVoltage >= MINOR_IMBALANCE)
  {
    batteryHealth = "MINOR IMBALANCE";
    batteryHealthCode = 1;
  }
  else
  {
    batteryHealth = "HEALTHY";
    batteryHealthCode = 0;
  }
}

// ==========================================================
// PROTECTION STATUS
// ==========================================================

void determineProtectionStatus()
{
  if (batteryHealth == "PACK FAILURE")
  {
    protectionStatus = "SHUTDOWN";
    protectionStatusCode = 3;
  }
  else if (batteryHealth == "CRITICAL")
  {
    protectionStatus = "FAILSAFE";
    protectionStatusCode = 2;
  }
  else if (batteryHealth == "MINOR IMBALANCE")
  {
    protectionStatus = "DEGRADED";
    protectionStatusCode = 1;
  }
  else
  {
    protectionStatus = "NORMAL";
    protectionStatusCode = 0;
  }
}

// ==========================================================
// FAULT HISTORY
// ==========================================================

void addFaultHistory(String message)
{
  if (faultCount < 10)
  {
    faultHistory[faultCount] = message;
    faultCount++;
  }
  else
  {
    for (int i = 0; i < 9; i++)
    {
      faultHistory[i] = faultHistory[i + 1];
    }

    faultHistory[9] = message;
  }
}

void logHealthStateChange()
{
  if (previousLoggedHealth == "")
  {
    previousLoggedHealth = batteryHealth;

    Serial.println();
    Serial.println("====================================");
    Serial.println("FAULT HISTORY");
    Serial.println("====================================");

    Serial.print("INITIAL STATE: ");
    Serial.println(batteryHealth);

    Serial.println("====================================");

    return;
  }

  if (batteryHealth != previousLoggedHealth)
  {
    String message =
      "STATE CHANGE: " +
      previousLoggedHealth +
      " -> " +
      batteryHealth;

    addFaultHistory(message);

    Serial.println();
    Serial.println("====================================");
    Serial.println("FAULT HISTORY");
    Serial.println("====================================");

    for (int i = 0; i < faultCount; i++)
    {
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.println(faultHistory[i]);
    }

    Serial.println("====================================");

    previousLoggedHealth = batteryHealth;
  }
}

// ==========================================================
// BLYNK TELEMETRY
// ==========================================================

void sendBlynkTelemetry()
{
  if (!Blynk.connected())
  {
    cloudSyncPending = true;

    Serial.println("Blynk unavailable. Telemetry remains queued.");

    return;
  }

  Serial.println();
  Serial.println("====================================");
  Serial.println("BLYNK TELEMETRY UPDATE");
  Serial.println("====================================");

  Blynk.virtualWrite(V0, cellVoltage[0]);
  Blynk.virtualWrite(V1, cellVoltage[1]);
  Blynk.virtualWrite(V2, cellVoltage[2]);
  Blynk.virtualWrite(V3, cellVoltage[3]);

  Blynk.virtualWrite(V4, averageVoltage);
  Blynk.virtualWrite(V5, imbalancePercent);

  Blynk.virtualWrite(V6, batteryHealth);
  Blynk.virtualWrite(V7, protectionStatus);

  Blynk.virtualWrite(V8, batteryHealthCode);
  Blynk.virtualWrite(V9, protectionStatusCode);

  Serial.print("Cell 1: ");
  Serial.print(cellVoltage[0], 2);
  Serial.println(" V");

  Serial.print("Cell 2: ");
  Serial.print(cellVoltage[1], 2);
  Serial.println(" V");

  Serial.print("Cell 3: ");
  Serial.print(cellVoltage[2], 2);
  Serial.println(" V");

  Serial.print("Cell 4: ");
  Serial.print(cellVoltage[3], 2);
  Serial.println(" V");

  Serial.print("Average: ");
  Serial.print(averageVoltage, 2);
  Serial.println(" V");

  Serial.print("Imbalance: ");
  Serial.print(imbalancePercent, 2);
  Serial.println(" %");

  Serial.print("Battery Health: ");
  Serial.println(batteryHealth);

  Serial.print("Health Code: ");
  Serial.println(batteryHealthCode);

  Serial.print("Protection Status: ");
  Serial.println(protectionStatus);

  Serial.print("Protection Code: ");
  Serial.println(protectionStatusCode);

  Serial.println("====================================");
}

// ==========================================================
// EVENT-DRIVEN TELEMETRY HANDLER
// ==========================================================

void handleCloudTelemetry()
{
  bool eventDetected = false;

  for (int i = 0; i < 4; i++)
  {
    if (previousCellVoltage[i] < -50.0)
    {
      eventDetected = true;
    }
    else if (
      fabs(cellVoltage[i] - previousCellVoltage[i])
      >= CLOUD_VOLTAGE_CHANGE
    )
    {
      eventDetected = true;
    }
  }

  if (previousAverageVoltage < -50.0)
  {
    eventDetected = true;
  }
  else if (
    fabs(averageVoltage - previousAverageVoltage)
    >= CLOUD_VOLTAGE_CHANGE
  )
  {
    eventDetected = true;
  }

  if (previousImbalancePercent < -50.0)
  {
    eventDetected = true;
  }
  else if (
    fabs(imbalancePercent - previousImbalancePercent)
    >= CLOUD_IMBALANCE_CHANGE
  )
  {
    eventDetected = true;
  }

  if (previousBatteryHealth == "")
  {
    eventDetected = true;
  }
  else if (batteryHealth != previousBatteryHealth)
  {
    eventDetected = true;
  }

  if (previousProtectionStatus == "")
  {
    eventDetected = true;
  }
  else if (protectionStatus != previousProtectionStatus)
  {
    eventDetected = true;
  }

  for (int i = 0; i < 4; i++)
  {
    previousCellVoltage[i] = cellVoltage[i];
  }

  previousAverageVoltage = averageVoltage;
  previousImbalancePercent = imbalancePercent;

  previousBatteryHealth = batteryHealth;
  previousProtectionStatus = protectionStatus;

  if (eventDetected)
  {
    cloudSyncPending = true;
  }

  if (Blynk.connected() && cloudSyncPending)
  {
    sendBlynkTelemetry();

    cloudSyncPending = false;
  }
}

// ==========================================================
// WIFI + BLYNK STATUS
// ==========================================================

void checkConnectionStatus()
{
  unsigned long currentTime = millis();

  if (currentTime - lastWiFiCheck >= WIFI_CHECK_INTERVAL)
  {
    lastWiFiCheck = currentTime;

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Wi-Fi disconnected.");
      Serial.println("Attempting Wi-Fi reconnect...");

      WiFi.reconnect();

      cloudSyncPending = true;
    }
  }

  bool currentlyConnected = Blynk.connected();

  if (currentlyConnected && !wasBlynkConnected)
  {
    Serial.println();
    Serial.println("====================================");
    Serial.println("BLYNK CONNECTION RESTORED");
    Serial.println("====================================");

    wasBlynkConnected = true;
    cloudSyncPending = true;
  }

  if (!currentlyConnected && wasBlynkConnected)
  {
    Serial.println();
    Serial.println("BLYNK CONNECTION LOST");

    wasBlynkConnected = false;
    cloudSyncPending = true;
  }

  if (currentlyConnected && cloudSyncPending)
  {
    sendBlynkTelemetry();

    cloudSyncPending = false;
  }
}

// ==========================================================
// SERIAL DIAGNOSTICS
// ==========================================================

void printDiagnostics()
{
  Serial.println();
  Serial.println("====================================");
  Serial.println("BATTERY INTELLIGENCE DIAGNOSTICS");
  Serial.println("====================================");

  for (int i = 0; i < 4; i++)
  {
    Serial.print("Cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(cellVoltage[i], 2);
    Serial.println(" V");
  }

  Serial.print("Average: ");
  Serial.print(averageVoltage, 2);
  Serial.println(" V");

  Serial.print("Minimum: ");
  Serial.print(minimumVoltage, 2);
  Serial.println(" V");

  Serial.print("Maximum: ");
  Serial.print(maximumVoltage, 2);
  Serial.println(" V");

  Serial.print("Imbalance: ");
  Serial.print(imbalanceVoltage, 2);
  Serial.println(" V");

  Serial.print("Imbalance: ");
  Serial.print(imbalancePercent, 2);
  Serial.println(" %");

  Serial.print("Weakest Cell: C");
  Serial.println(weakestCell + 1);

  Serial.print("Strongest Cell: C");
  Serial.println(strongestCell + 1);

  Serial.print("Battery Health: ");
  Serial.println(batteryHealth);

  Serial.print("Health Code: ");
  Serial.println(batteryHealthCode);

  Serial.print("Protection Status: ");
  Serial.println(protectionStatus);

  Serial.print("Protection Code: ");
  Serial.println(protectionStatusCode);

  Serial.print("Wi-Fi RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.print("Blynk: ");
  Serial.println(Blynk.connected() ? "CONNECTED" : "DISCONNECTED");

  Serial.println("====================================");
}

// ==========================================================
// SETUP
// ==========================================================

void setup()
{
  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("====================================");
  Serial.println("ADAPTIVE MULTI-CELL BMS");
  Serial.println("ESP32 + BLYNK");
  Serial.println("====================================");

  analogReadResolution(12);

  Serial.println("Connecting to Wi-Fi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long wifiStart = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - wifiStart < 15000
  )
  {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wi-Fi CONNECTED");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
  else
  {
    Serial.println("Wi-Fi connection not established.");
  }

  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.println("Connecting to Blynk...");

  if (Blynk.connect(10000))
  {
    Serial.println("BLYNK CONNECTED");

    wasBlynkConnected = true;
    cloudSyncPending = true;
  }
  else
  {
    Serial.println("BLYNK CONNECTION FAILED");
    Serial.println("System will continue and retry.");
  }

  readCellVoltages();
  calculateBatteryParameters();
  determineBatteryHealth();
  determineProtectionStatus();

  logHealthStateChange();

  Serial.println();
  Serial.println("BMS SYSTEM READY");
}

// ==========================================================
// MAIN LOOP
// ==========================================================

void loop()
{
  unsigned long currentTime = millis();

  Blynk.run();

  checkConnectionStatus();

  if (currentTime - lastSensorRead >= SENSOR_INTERVAL)
  {
    lastSensorRead = currentTime;

    readCellVoltages();
    calculateBatteryParameters();
    determineBatteryHealth();
    determineProtectionStatus();

    logHealthStateChange();
    handleCloudTelemetry();
  }

  if (currentTime - lastSerialPrint >= SERIAL_INTERVAL)
  {
    lastSerialPrint = currentTime;

    printDiagnostics();
  }
}
