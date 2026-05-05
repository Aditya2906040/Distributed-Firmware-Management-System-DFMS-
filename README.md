# Distributed Firmware Management System (DFMS)

## Overview

The Distributed Firmware Management System (DFMS) is a system-level implementation for reliable, scalable, and fault-tolerant firmware updates across multiple embedded devices.

It is designed to manage the complete firmware lifecycle — including versioning, distribution, validation, rollback, and staged deployment — across distributed nodes such as ESP32 and ESP8266 devices.

---

## Key Features

* Local OTA updates via device-hosted web interface
* HTTP-based remote firmware updates
* Dual-partition (A/B) firmware architecture
* Automatic rollback on failure (power loss, crash, invalid firmware)
* Post-boot validation mechanism
* Firmware integrity verification (size + checksum)
* Multi-device polling-based update system
* Multi-MCU support (ESP32, ESP8266; extensible)
* Staged rollout (canary deployment)
* Update status reporting and centralized monitoring

---

## System Architecture

### High-Level Architecture

* Devices periodically poll the server for update decisions
* Server responds with firmware metadata (version, URL, checksum)
* Devices download firmware and perform OTA updates
* Devices report update status back to the server
* Rollout is controlled centrally using canary deployment strategy

---

### Device Architecture

The device is structured into modular components:

* **OTA State Machine** – Controls the OTA lifecycle
* **OTA Manager** – Handles firmware installation and validation
* **Network Manager** – Manages WiFi and HTTP communication
* **Version Manager** – Tracks and compares firmware versions
* **Flash Manager** – Handles A/B partition writes
* **NVS Storage** – Stores persistent state (version, flags, status)

---

## OTA Lifecycle

```
IDLE
  ↓
CHECK_FOR_UPDATE
  ↓
DOWNLOAD
  ↓
VERIFY
  ↓
INSTALL (inactive partition)
  ↓
REBOOT
  ↓
POST-BOOT VALIDATION
   ├── SUCCESS → MARK VALID
   └── FAILURE → ROLLBACK
```

---

## Firmware Safety Model

### Dual Partition (A/B)

* Firmware is written to an inactive partition
* Current firmware remains untouched during update
* Bootloader switches partitions only after successful installation

---

### Failure Handling

The system handles the following failure scenarios:

* **Power Loss During Update**
  Active firmware remains intact

* **Corrupted Firmware**
  Detected during verification, update aborted

* **Runtime Failure (Crash / Watchdog Reset)**
  Firmware not validated → automatic rollback

---

## Post-Boot Validation

After reboot, the new firmware enters a *pending verification state*.

The firmware must:

* Initialize successfully
* Complete basic runtime checks
* Explicitly mark itself as valid

If validation fails:

* The system rolls back to the previous firmware

---

## Firmware Integrity

Before installation:

* Firmware size is checked against partition size
* Checksum verification ensures data integrity

---

## Flash Memory (Concept)

Flash memory is a non-volatile storage medium used in embedded systems to store firmware, configuration data, and persistent state.

Unlike RAM, flash retains data even when power is removed. However, it has specific characteristics:

* Data is written in blocks (not byte-wise freely)
* Erase operations occur at sector level
* Limited write/erase cycles (wear considerations)
* Writes are slower than reads

In ESP32, flash memory stores:

* Bootloader
* Application firmware
* OTA partitions
* Persistent storage (NVS)

Understanding flash memory is essential for designing reliable OTA systems.

---

## ESP32 Flash Memory Layout (OTA Configuration)

A typical ESP32 with 4MB flash uses a partition table to divide memory.

### Example Layout

```
| Bootloader | Partition Table | NVS | OTA_0 | OTA_1 |
```

### Description

* **Bootloader**
  Initializes system and selects which partition to boot

* **Partition Table**
  Defines memory regions and sizes

* **NVS (Non-Volatile Storage)**
  Stores configuration, flags, and OTA state

* **OTA_0 / OTA_1 (A/B Partitions)**
  Two firmware slots used for safe updates

---

### Key Concept: A/B Partitioning

* Device runs firmware from one partition (e.g., OTA_0)
* New firmware is written to inactive partition (OTA_1)
* Bootloader switches only after success
* On failure, system rolls back automatically

---

### Size Constraint

Firmware must fit within a single OTA partition, not total flash.

Example:

* Total flash: 4MB
* OTA partition: ~1.3MB

👉 Firmware must be ≤ partition size

---

## Multi-Device Update Model

* Devices periodically poll the server

* Each device sends:

  * Device ID
  * Device type
  * Current firmware version

* Server decides:

  * Whether update is required
  * Which firmware to provide

---

## Multi-MCU Support

* Devices identify themselves using metadata
* Server delivers firmware based on device type
* Supports ESP32 and ESP8266 (extensible to other platforms)

---

## Staged Rollout (Canary Deployment)

* Firmware is first deployed to a subset of devices
* Devices report update success or failure
* Server evaluates results
* Full rollout occurs only after successful validation

---

## Project Phases

### Phase 1 — Local OTA

Basic OTA using device-hosted web interface.

### Phase 2 — HTTP OTA (Single Device)

Remote firmware update using server-based version check.

### Phase 3 — Dual Partition Setup

Implementation of A/B partitioning for safe updates.

### Phase 4 — Failure Detection & Recovery

Rollback mechanism for failed updates and power interruptions.

### Phase 5 — Post-Boot Validation & Reporting

Validation of firmware after reboot and reporting to server.

### Phase 6 — Integrity Validation

Verification using checksum and size checks.

### Phase 7 — Multi-Device Polling

Distributed update system using periodic device polling.

### Phase 8 — Multi-MCU Support

Support for multiple device types with server-side control.

### Phase 9 — Staged Rollout

Controlled deployment using canary strategy.

### Phase 10 — Monitoring & Logging

Tracking update status and system behavior.

### Phase 11 (Optional) — Security Layer

HTTPS support and firmware authenticity verification.

---

## Future Scope

* Secure boot integration
* Firmware signing and verification
* Delta firmware updates
* MQTT-based update triggers
* Centralized monitoring dashboard

---

## Learning Outcomes

* Embedded system reliability design
* OTA firmware lifecycle management
* Distributed system coordination
* Backend + embedded system integration
* Deployment and rollback strategies

---

## Summary

DFMS is designed as a firmware lifecycle management system rather than a basic OTA implementation, focusing on reliability, scalability, and controlled deployment across distributed embedded devices.

---

## Author

**Aditya Raman**

Embedded Systems | IoT | Systems Programming

* Focus Areas: Embedded Systems, Networking, Firmware Design
* Tech Stack: C/C++, ESP32, Arduino, Socket Programming, Git

---

### Contact

* GitHub: https://github.com/<your-username>
* LinkedIn: https://linkedin.com/in/<your-profile>

---
