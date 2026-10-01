# Interactive BMS Web Demonstration

This folder contains the browser-based interactive demonstration for the Adaptive Multi-Cell Battery Intelligence Engine.

## Run locally
Open `index.html` in a browser.

## Deploy
The folder can be deployed as a static site on Render. No server-side runtime is required.

## Demonstration
The four sliders represent simulated cell voltages from 2.50 V to 4.30 V. The page calculates:
- Cell voltages
- Average voltage
- Cell imbalance
- Weakest and strongest cells
- Battery health state
- Protection state
- Relay and buzzer status

The browser simulation is a demonstration of the BMS decision logic. The physical implementation uses ESP32 inputs and Blynk telemetry.

## Security
No Wi-Fi password, Blynk authentication token, API key, or other secret is stored in this web demo.
