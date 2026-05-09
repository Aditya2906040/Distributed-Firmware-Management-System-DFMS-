# DFMS — Local OTA Partition Exploration Notes

## Overview

This document contains detailed notes and observations from the local OTA partition exploration phase of the DFMS project.

The goal of this phase was to understand how ESP-IDF OTA systems work internally before introducing networking or HTTP-based OTA updates.

Instead of treating OTA as a simple firmware download process, this phase focused on understanding:

- Flash partition architecture
- Bootloader behavior
- OTA slot management
- Firmware image validation
- Boot partition switching
- OTA metadata handling
- OTA image lifecycle states
- Difference between raw flashing and OTA-managed updates

This phase established the foundational understanding required for reliable firmware update systems.

---

# Initial Partition Table

The ESP32 was configured with an OTA-enabled partition table.

Observed partition table:

```text
nvs
otadata
phy_init
factory
ota_0
ota_1
```

---

# Understanding Each Partition

| Partition | Purpose |
|---|---|
| nvs | Non-volatile storage |
| otadata | OTA metadata and boot selection |
| phy_init | RF calibration data |
| factory | Factory firmware image |
| ota_0 | OTA firmware slot A |
| ota_1 | OTA firmware slot B |

---

# OTA Architecture Insight

The OTA architecture follows an:

```text
A/B firmware model
```

Meaning:

- one firmware slot is currently active
- another firmware slot is used for updates
- firmware updates never overwrite the currently running firmware

This significantly improves reliability.

---

# Initial OTA Awareness

The firmware first explored:

```c
esp_ota_get_running_partition()
```

and:

```c
esp_ota_get_next_update_partition(NULL)
```

---

# Running Partition Detection

The following output was initially observed:

```text
Running partition: factory
Next update partition: ota_0
```

---

# Important Understanding

This revealed that:

- firmware can detect where it is currently executing from
- ESP-IDF automatically determines the next suitable OTA slot
- OTA slots alternate between ota_0 and ota_1

---

# Why This Matters

OTA systems must never overwrite:

```text
currently running firmware
```

Otherwise:

- interrupted updates
- power failures
- corrupted firmware

could permanently brick the device.

The OTA subsystem therefore always chooses:

```text
inactive OTA slot
```

for updates.

---

# Boot Slot Alternation

Observed OTA slot alternation behavior:

| Current Running | Next Update Partition |
|---|---|
| factory | ota_0 |
| ota_0 | ota_1 |
| ota_1 | ota_0 |

This demonstrated the OTA A/B switching mechanism.

---

# Exploring OTA Image States

The following API was explored:

```c
esp_ota_get_state_partition()
```

along with:

```c
esp_ota_img_states_t
```

---

# OTA Image States

ESP-IDF internally tracks firmware lifecycle states.

Important OTA image states include:

| State | Meaning |
|---|---|
| ESP_OTA_IMG_VALID | trusted firmware |
| ESP_OTA_IMG_PENDING_VERIFY | firmware under validation |
| ESP_OTA_IMG_INVALID | invalid firmware |
| ESP_OTA_IMG_ABORTED | failed verification |
| ESP_OTA_IMG_UNDEFINED | no OTA lifecycle metadata |

---

# Factory Partition Behavior

When attempting to retrieve OTA state from the factory partition:

```text
Failed to retrieve OTA state
```

was observed.

---

# Important Insight

The factory partition does not fully participate in the OTA lifecycle state machine.

The factory image is treated more like:

- baseline firmware
- recovery firmware
- initial stable firmware

rather than a dynamically managed OTA image.

This explained why OTA lifecycle metadata was unavailable for the factory partition.

---

# Boot Partition Switching Exploration

The following API was explored:

```c
esp_ota_set_boot_partition()
```

---

# Purpose of esp_ota_set_boot_partition()

This API changes:

```text
future boot target
```

by updating:

```text
otadata partition
```

The API does NOT:

- copy firmware
- write application image
- perform OTA transfer

Instead, it only changes:

```text
which partition bootloader should load next
```

---

# Initial Boot Switching Attempt

The first attempt produced:

```text
image at 0x110000 has invalid magic byte
```

---

# Understanding the Error

This error occurred because:

```text
ota_0 was empty
```

No valid firmware image existed inside the OTA slot.

---

# Magic Byte Concept

ESP32 firmware images begin with:

```text
special identification bytes
```

called:

```text
magic bytes
```

These allow ESP-IDF and the bootloader to verify:

```text
whether a flash region contains a valid firmware image
```

---

# Important Reliability Behavior

ESP-IDF refused to switch boot partition because:

```text
boot target firmware was invalid
```

This demonstrated that ESP-IDF performs:

- firmware image validation
- safety checks before boot switching

This prevents devices from booting into empty or corrupted partitions.

---

# Manual OTA Firmware Placement

To continue experimentation, firmware was manually written into:

```text
ota_0
```

using:

```bash
esptool.py --chip esp32 \
--port /dev/ttyUSB0 \
write_flash 0x110000 build/dfms.bin
```

---

# Important Observation

This operation:

- only wrote raw firmware bytes into flash
- did not use ESP-IDF OTA lifecycle APIs
- did not create OTA metadata
- did not create validation states

---

# Firmware Variant Experiment

Two firmware variants were intentionally created.

---

# Factory Firmware

Behavior:

- 1000ms LED blink
- printed:

```text
FACTORY firmware
```

---

# OTA_0 Firmware

Behavior:

- 500ms LED blink
- printed:

```text
OTA_0 firmware
```

---

# Purpose of Behavioral Differences

This made firmware transitions visually observable.

The experiment demonstrated:

- runtime firmware identity
- partition switching confirmation
- successful OTA slot booting

This approach closely resembles real-world OTA debugging strategies.

---

# Successful OTA Slot Boot

After manually placing firmware into ota_0 and switching boot partition, the following output was observed:

```text
Running partition: ota_0
Next update partition: ota_1
```

---

# Major Milestone Achieved

This confirmed:

- bootloader correctly read otadata
- boot partition switching worked
- firmware image validation succeeded
- OTA slot loading worked correctly
- A/B OTA architecture functioned as expected

---

# OTA State Retrieval from ota_0

After booting from ota_0:

```text
Firmware state: UNDEFINED
```

was observed.

---

# Extremely Important Insight

This became one of the most important learnings of the exploration.

Although the firmware image was valid and bootable:

```text
OTA lifecycle metadata was still missing
```

---

# Why OTA State Was UNDEFINED

The firmware had been written using:

```text
raw flash writing via esptool
```

instead of:

```text
ESP-IDF OTA APIs
```

Therefore:

- firmware physically existed
- bootloader could execute it
- but OTA lifecycle metadata had never been initialized

---

# Raw Flashing vs OTA-Managed Installation

This exploration revealed a critical architectural distinction.

---

# Raw Flashing

Using:

```bash
esptool.py write_flash
```

performs:

- raw byte writing into flash

but does NOT:

- manage OTA states
- manage rollback metadata
- initialize validation lifecycle
- mark firmware pending verification
- create OTA trust states

---

# OTA-Managed Installation

Real OTA APIs such as:

```c
esp_ota_begin()
esp_ota_write()
esp_ota_end()
esp_ota_set_boot_partition()
```

perform BOTH:

- firmware writing
- OTA lifecycle management

This includes:

- metadata initialization
- rollback support
- verification states
- OTA trust management

---

# Major Architectural Realization

One of the most important discoveries was:

```text
OTA is NOT merely firmware flashing
```

Reliable OTA systems also require:

- metadata management
- lifecycle tracking
- image validation
- rollback logic
- trust state management

---

# Understanding otadata

The otadata partition acts as OTA control metadata.

It stores information such as:

- currently selected boot slot
- active OTA image
- rollback information
- firmware validation state
- OTA lifecycle metadata

The bootloader reads otadata during boot.

---

# ESP32 Boot Flow Revisited

The OTA exploration reinforced understanding of the ESP32 boot process:

```text
ROM Bootloader
    ↓
2nd Stage Bootloader
    ↓
Partition Table Read
    ↓
Read otadata
    ↓
Select Boot Partition
    ↓
Validate Firmware Image
    ↓
Load Firmware
    ↓
FreeRTOS Startup
    ↓
app_main()
```

---

# Key Engineering Lessons Learned

## 1. OTA Is Primarily Lifecycle Management

OTA involves much more than firmware transfer.

Reliable OTA systems require:

- boot management
- metadata tracking
- validation logic
- rollback handling

---

## 2. Firmware Placement and Firmware Lifecycle Are Different

A firmware image may:

- physically exist in flash
- boot successfully

while still lacking:

```text
OTA lifecycle state metadata
```

---

## 3. Bootloader Performs Safety Validation

ESP-IDF validates firmware images before allowing boot partition switching.

Invalid or empty OTA slots are rejected automatically.

---

## 4. Factory and OTA Partitions Behave Differently

Factory firmware behaves more like:

- stable baseline image
- recovery image

while OTA partitions participate in:

- validation
- rollback
- trust state management

---

## 5. A/B OTA Architecture Improves Reliability

Firmware updates alternate between:

```text
ota_0 ↔ ota_1
```

This ensures:

- current firmware remains intact
- rollback remains possible
- interrupted updates are recoverable

---

## 6. Bootloader and otadata Are Central to OTA Systems

The bootloader determines:

- which firmware boots
- whether firmware is trusted
- whether rollback occurs

using information stored inside:

```text
otadata
```

---

# Current Status of DFMS OTA Exploration

Successfully explored:

- OTA partition tables
- running partition detection
- inactive OTA slot selection
- boot partition switching
- firmware image validation
- manual firmware placement
- OTA slot alternation
- OTA image states
- raw flashing vs OTA-managed installation

---

# Planned Next Phase

The next phase will focus on:

- true OTA write lifecycle
- esp_ota_begin()
- esp_ota_write()
- esp_ota_end()
- firmware validation states
- pending verification flow
- rollback behavior
- OTA metadata creation

Initially this will still be explored locally before introducing networking.

---

# Final Reflection

This exploration phase demonstrated that:

```text
OTA systems are fundamentally boot-management and lifecycle-management systems.
```

Networking is only responsible for:

```text
delivering firmware data
```

while the true complexity of OTA lies in:

- bootloader interaction
- partition management
- firmware trust lifecycle
- validation logic
- rollback safety

Understanding these concepts before implementing network OTA greatly improves architectural understanding and debugging capability.

