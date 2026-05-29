# Drone Atmospheric Monitoring Project 

This is an **R&D project funded by a AADF/READ grant** to build a **mobile atmospheric monitoring system** using a drone platform. The system architecture consists of **three measurement nodes**:

* **1 drone-mounted monitoring station**
* **1 fixed ground reference station**
* **1 mobile ground monitoring station** 

The drone platform is a **DJI Matrice 350 RTK**.



## Project Goal

Develop a **low-cost environmental monitoring system** capable of:

* mapping **air-pollution hotspots**
* performing **vertical atmospheric profiles**
* comparing drone measurements with **ground reference stations**

The system focuses on **relative air-quality indices rather than regulatory-grade concentrations**, to reduce integration complexity and cost.



## Sensor Strategy

Instead of multiple individual sensors (PM + gas + environmental), the system uses a **single integrated environmental node**:

**Primary sensor**

* **Sensirion SEN55**

    * PM1 / PM2.5 / PM4 / PM10
    * Temperature
    * Humidity
    * VOC index
    * NOx index
    * I²C interface

Advantages:

* factory calibrated
* digital output
* minimal analog electronics
* easier integration and maintenance

Optional upgrades discussed:

* **SCD4x CO₂ sensor**
* **SPS30 PM sensor for cross-validation**



## Electronics Architecture

Each station uses the same electronics design.

Core components:

**Microcontroller**

* ESP32-S3-DevKitC-1U development board

**Positioning**

* u-blox NEO-M9N GNSS module

**Communication**

* LoRa (868 MHz) telemetry

**Data storage**

* microSD logging

**Power**

* LiPo battery (drone/mobile)
* DC-DC regulators



## Communication Architecture

Telemetry uses **point-to-point LoRa (868 MHz)**.

Structure:

Drone node
→ LoRa
→ Ground station receiver

A LoRaWAN gateway is optional for later expansion.



## Airflow and Sensor Placement

Drone airflow strongly affects measurements.

Recommended design:

* **Side boom mounting**
* sensor inlet placed **~0.7–1.0 m from drone center**
* **aspirated sampling inlet** using a small fan
* **short inlet tubing**

This minimizes rotor downwash interference.



## Calibration Strategy

Because the system uses **relative air-quality indices**, calibration relies on **co-location** instead of expensive gas calibrators.

Procedure:

1. Run all stations **side-by-side for 1–2 weeks**
2. Compare sensor outputs
3. derive correction factors
4. validate against any available **reference station**

Industrial calibration devices exist but are **too expensive** for this project.



## Flight Sampling Protocol

Recommended drone measurement strategy:

1. Fly to location
2. **hover sampling segments (10–30 s)**
3. move to next point
4. repeat

Avoid measuring during rapid motion.

Use:

* vertical profiles
* grid sampling
* repeated transects



## System Deployment

Final system will include:

**Drone station**

* SEN55
* ESP32
* GNSS
* LoRa
* microSD
* aspirated inlet

**Fixed ground station**

* same sensors
* weatherproof enclosure
* continuous operation

**Mobile ground station**

* same hardware
* portable enclosure



