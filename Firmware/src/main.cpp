#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CSen5x.h>
#include <math.h>

// ESP32-S3-DevKitC-1 pins you are currently using
constexpr uint8_t I2C_SDA_PIN = 8;
constexpr uint8_t I2C_SCL_PIN = 9;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t I2C_CLOCK_HZ = 100000;   // SEN55 max: 100 kHz standard mode
constexpr uint8_t SEN55_I2C_ADDR = 0x69;

SensirionI2CSen5x sen5x;

void printSensirionError(const char* operation, uint16_t error)
{
    char errorMessage[256];
    errorToString(error, errorMessage, sizeof(errorMessage));

    Serial.print("[SEN55] ");
    Serial.print(operation);
    Serial.print(" failed: ");
    Serial.println(errorMessage);
}

bool scanForSen55()
{
    Serial.println();
    Serial.println("[I2C] Scanning bus...");

    bool found = false;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t result = Wire.endTransmission();

        if (result == 0) {
            Serial.print("[I2C] Found device at 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);

            if (address == SEN55_I2C_ADDR) {
                found = true;
            }
        }
    }

    if (!found) {
        Serial.println("[I2C] SEN55 not found at address 0x69.");
    } else {
        Serial.println("[I2C] SEN55 detected at address 0x69.");
    }

    return found;
}

void setup()
{
    Serial.begin(SERIAL_BAUD);
    delay(1500);

    Serial.println();
    Serial.println("Standalone SEN55 test");
    Serial.println("---------------------");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_CLOCK_HZ);

    Serial.print("[I2C] SDA = GPIO");
    Serial.println(I2C_SDA_PIN);
    Serial.print("[I2C] SCL = GPIO");
    Serial.println(I2C_SCL_PIN);
    Serial.print("[I2C] Clock = ");
    Serial.print(I2C_CLOCK_HZ);
    Serial.println(" Hz");

    bool sen55AddressFound = scanForSen55();

    if (!sen55AddressFound) {
        Serial.println();
        Serial.println("[STOP] Sensor not visible on I2C.");
        Serial.println("Check: 5V power, GND common with ESP32, SDA/SCL order, pull-ups, SEL to GND.");
        return;
    }

    sen5x.begin(Wire);

    uint16_t error;

    error = sen5x.deviceReset();
    if (error) {
        printSensirionError("deviceReset", error);
        return;
    }

    delay(200);

    float tempOffset = 0.0f;
    error = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (error) {
        printSensirionError("setTemperatureOffsetSimple", error);
        Serial.println("[SEN55] Continuing anyway; temperature offset is not essential.");
    } else {
        Serial.println("[SEN55] Temperature offset set to 0.0 C");
    }

    error = sen5x.startMeasurement();
    if (error) {
        printSensirionError("startMeasurement", error);
        return;
    }

    Serial.println("[SEN55] Measurement started.");
    Serial.println("[SEN55] Waiting 3 seconds before first read...");
    delay(3000);

    Serial.println();
    Serial.println("t_ms,pm1p0,pm2p5,pm4p0,pm10p0,humidity,temperature,voc_index,nox_index");
}

void loop()
{
    static uint32_t lastReadMs = 0;
    const uint32_t now = millis();

    if (now - lastReadMs < 1000) {
        return;
    }

    lastReadMs = now;

    float pm1p0 = NAN;
    float pm2p5 = NAN;
    float pm4p0 = NAN;
    float pm10p0 = NAN;
    float humidity = NAN;
    float temperature = NAN;
    float vocIndex = NAN;
    float noxIndex = NAN;

    uint16_t error = sen5x.readMeasuredValues(
        pm1p0,
        pm2p5,
        pm4p0,
        pm10p0,
        humidity,
        temperature,
        vocIndex,
        noxIndex
    );

    if (error) {
        printSensirionError("readMeasuredValues", error);

        // Re-scan after a failed read. This tells us whether the device disappeared
        // from the I2C bus or only the high-level SEN55 command failed.
        scanForSen55();
        return;
    }

    Serial.print(now);
    Serial.print(",");
    Serial.print(pm1p0, 2);
    Serial.print(",");
    Serial.print(pm2p5, 2);
    Serial.print(",");
    Serial.print(pm4p0, 2);
    Serial.print(",");
    Serial.print(pm10p0, 2);
    Serial.print(",");
    Serial.print(humidity, 2);
    Serial.print(",");
    Serial.print(temperature, 2);
    Serial.print(",");
    Serial.print(vocIndex, 0);
    Serial.print(",");
    Serial.println(noxIndex, 0);
}
