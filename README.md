Team Crum-b Salt Pack Dispenser instructions:


Inside CAD Parts, STL files included used for 3D printing. Made with restrictive bounding sizes due to Hackathon, total size is 7400 mm x 5100 mm x 2400 mm, restrictions made to fit into 200mm x 200mm x 200mm. Infill required 30% minimum, 50% suggested. Building watertight seals, connection friendly sockets between pieces WIP.

Required Parts:

Compute & Sensing

ESP-32 ×1 — reads GPS position, sends it to the backend, drives the rest of the electronics
GY-NEO6MV2 (NEO-6M GPS + ceramic antenna) ×1
DHT11 ×1 — basic temperature / humidity sensing
Power

18650 Li-Ion holders (1/2/4-cell) ×2
LM2596 buck converters ×2 — step down for logic-level supply
MT3608 boost converters ×2 — step up for motor/actuator drive voltage
Motor & Actuator Drivers

L298N motor drivers ×2
A4988 / DRV8825 stepper drivers ×1
12V solenoids (push/pull) ×1
Passives

100nF capacitors ×8
10µF capacitors ×4
2.2µF capacitors ×2
1µF capacitors ×2
22pF capacitors ×2
Wiring

Jumper wires (M-M, M-F, F-F) ×10
Breadboard jumper wire (100mm) ×10

Within WiFi stuff, the data sends packets between multiple ESP-32 communications and to a backend data server. Server can be ran through terminal on VS Code.
XGBoost Model Classifier MUST be ran separately on another server; this predicts chance of snow using real-time data sent from packets of the mobile units.

