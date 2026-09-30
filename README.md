# Adaptive Multi-Cell Battery Intelligence Engine

## Project Overview

The Adaptive Multi-Cell Battery Intelligence Engine is an embedded battery monitoring and protection system designed to monitor a multi-cell battery pack in real time.

The system monitors individual cell voltages, calculates the average pack voltage and cell imbalance, detects abnormal battery conditions, and determines the overall battery health and protection state.

The project combines ESP32-based embedded monitoring, simulated multi-cell inputs, fault detection, event-driven cloud telemetry, and a Blynk dashboard for live visualization.

## Key Features

- Four-cell battery monitoring
- Individual cell voltage monitoring
- Average battery voltage calculation
- Cell imbalance calculation
- Weakest and strongest cell identification
- Battery health classification
- Fault and abnormal-condition detection
- Protection-state management
- NORMAL, DEGRADED, FAILSAFE and SHUTDOWN states
- ESP32-based embedded implementation
- Wi-Fi connectivity
- Event-driven Blynk cloud telemetry
- Live battery dashboard
- Live graphs and gauges
- Fault and recovery testing
- Modular and scalable design approach

## System States

| State | Description |
|---|---|
| NORMAL | Battery operating within healthy limits |
| DEGRADED | Minor abnormality or imbalance detected |
| FAILSAFE | Critical battery condition detected |
| SHUTDOWN | Severe/pack-failure condition requiring protection |

## Technologies Used

- ESP32
- Arduino/C++
- Tinkercad
- Blynk IoT
- Wi-Fi
- Analog sensor inputs
- Embedded state-machine logic

## Battery Monitoring

The system continuously evaluates:

- Cell 1 voltage
- Cell 2 voltage
- Cell 3 voltage
- Cell 4 voltage
- Average cell voltage
- Voltage imbalance
- Battery health
- Protection status

## Fault Testing

The system was tested under different operating conditions, including:

- Healthy battery condition
- Minor cell imbalance
- Critical cell imbalance
- Pack failure condition
- Recovery to healthy operation
- Undervoltage
- Overvoltage
- Sensor fault/disconnection conditions

## Cloud Dashboard

Blynk is used for live cloud telemetry and visualization.

The dashboard provides live battery information through numerical values, gauges and graphs, allowing the battery condition to be monitored remotely.

## Architecture

```text
Multi-Cell Battery Inputs
          |
          v
      ESP32 Controller
          |
          +--> Cell Voltage Monitoring
          |
          +--> Average & Imbalance Calculation
          |
          +--> Fault Detection
          |
          +--> Battery Health Classification
          |
          +--> Protection State Machine
          |
          v
      Wi-Fi Connectivity
          |
          v
      Blynk Cloud
          |
          v
   Live Dashboard / Graphs
