# ESP-IDF Fundamentals and OTA Foundations

## Overview

This document contains detailed notes and learnings gathered while exploring ESP-IDF and building the foundational understanding required for the Distributed Firmware Management System (DFMS) project.

The purpose of this document is not only to explain how ESP-IDF works, but also to understand the architectural concepts behind:

- Embedded firmware systems
- ESP32 boot process
- FreeRTOS integration
- Flash memory layout
- Partition tables
- OTA firmware architecture
- Firmware build systems
- Runtime initialization

This document acts as engineering notes for future DFMS development.

---

# Why ESP-IDF Instead of Arduino?

Initially, Arduino-style development appears simpler:

```text
setup()
loop()
```

However, the DFMS project requires:

- OTA firmware updates
- Rollback support
- Partition management
- Flash memory control
- Networking stack access
- Reliable multitasking
- Event-driven firmware
- Modular firmware architecture

Arduino abstracts most low-level details away.

ESP-IDF, on the other hand, exposes the actual embedded firmware architecture.

ESP-IDF provides:

- Native ESP32 APIs
- Integrated FreeRTOS
- Partition table management
- OTA APIs
- Bootloader support
- WiFi stack access
- HTTP client/server support
- Modular component architecture
- Better scalability for large projects

ESP-IDF behaves more like a professional embedded framework than a simple microcontroller SDK.

---

# ESP-IDF Project Creation

A new ESP-IDF project was created using:

```bash
idf.py create-project dfms
```

This generated the following structure:

```text
.
├── CMakeLists.txt
└── main
    ├── CMakeLists.txt
    └── dfms.c
```

At first glance, the project structure appears very small.

However, ESP-IDF internally provides:

- FreeRTOS
- WiFi stack
- TCP/IP stack
- Drivers
- Flash management
- Bootloader
- OTA framework
- Build system

The user application is only a small part of the overall firmware system.

---

# ESP-IDF Build Philosophy

Unlike Arduino, ESP-IDF is based on:

```text
CMake + Ninja
```

The firmware is built using a modular component-based architecture.

ESP-IDF applications are assembled from:

- User components
- ESP-IDF components
- Drivers
- Runtime libraries
- FreeRTOS
- Bootloader

This architecture is significantly more scalable than Arduino-style monolithic sketches.

---

# Understanding CMakeLists.txt

## Root CMakeLists.txt

The root `CMakeLists.txt` file:

```cmake
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(dfms)
```

### Important Observations

#### 1. ESP-IDF Build System Integration

```cmake
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
```

This line imports the ESP-IDF build framework.

This is where:

- build logic
- component handling
- compiler configuration
- linker setup
- firmware generation

are integrated into the project.

---

#### 2. Project Declaration

```cmake
project(dfms)
```

Defines:

- project identity
- firmware naming
- build naming

The final firmware binary is named based on this project name.

---

# Understanding main/CMakeLists.txt

The file:

```cmake
idf_component_register(SRCS "dfms.c"
                    INCLUDE_DIRS ".")
```

This line is extremely important.

---

# Component-Based Architecture

ESP-IDF treats every module as a:

```text
component
```

Examples of ESP-IDF components:

- WiFi
- OTA
- NVS
- HTTP server
- BLE
- GPIO drivers
- SPI drivers

The `main/` directory itself is also a component.

---

# Understanding idf_component_register()

## Source Registration

```cmake
SRCS "dfms.c"
```

This tells ESP-IDF:

```text
Compile dfms.c as part of this component
```

---

## Include Directories

```cmake
INCLUDE_DIRS "."
```

This adds the current directory to the compiler include path.

This allows:

```c
#include "my_header.h"
```

from within the component.

---

# app_main() vs main()

The generated source file contained:

```c
void app_main(void)
{

}
```

A major observation:

ESP-IDF does NOT use:

```c
int main()
```

Instead:

```c
app_main()
```

is used.

---

# Why app_main() Exists

ESP-IDF internally initializes:

- bootloader
- runtime
- memory
- scheduler
- FreeRTOS

before user code starts.

`app_main()` is launched as a FreeRTOS task.

This means:

```text
Even the simplest ESP-IDF application already runs on top of FreeRTOS
```

This is one of the most important architectural differences compared to Arduino.

---

# FreeRTOS Integration

ESP-IDF uses FreeRTOS internally.

This means:

- multitasking already exists
- scheduler already exists
- system tasks already exist
- networking tasks already exist

before the user application starts.

---

# Important Insight

In ESP-IDF:

```text
Firmware execution is scheduler-driven
```

rather than:

```text
single infinite loop execution
```

This becomes extremely important for:

- OTA
- networking
- background services
- timers
- event handling

---

# First Firmware Build

The project was built using:

```bash
idf.py build
```

This generated several important firmware artifacts.

---

# Important Build Outputs

## 1. bootloader.bin

Generated at:

```text
build/bootloader/bootloader.bin
```

This is the second-stage bootloader.

Responsibilities:

- initialize flash
- read partition table
- choose firmware partition
- load application firmware

This bootloader later becomes responsible for:

- OTA slot selection
- rollback logic
- recovery handling

---

## 2. partition-table.bin

Generated at:

```text
build/partition_table/partition-table.bin
```

This defines the flash memory layout.

The partition table tells ESP32:

```text
What exists in flash memory and where it exists
```

This is one of the most important concepts in OTA systems.

---

## 3. dfms.bin

Generated at:

```text
build/dfms.bin
```

This is the application firmware image.

It contains:

- user code
- linked libraries
- FreeRTOS
- drivers
- runtime sections

This firmware image later becomes:

```text
OTA update payload
```

---

# Incremental Build System

ESP-IDF builds are incremental.

This means:

- only modified files are rebuilt
- unchanged components remain cached

This significantly reduces build time after the first build.

---

# Firmware Development Workflow

The standard ESP-IDF workflow became:

```text
Edit
→ Build
→ Flash
→ Monitor
```

This is the standard firmware development cycle.

---

# First Hardware Experiment — LED Blink

A GPIO blink application was implemented using the onboard LED connected to GPIO2.

The firmware used:

- GPIO driver
- FreeRTOS delays
- serial logging

---

# Blink Firmware Code

```c
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_2

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(LED_GPIO, 1);
        printf("LED ON\n");

        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_GPIO, 0);
        printf("LED OFF\n");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

---

# GPIO Driver Concepts

## gpio_reset_pin()

Resets GPIO configuration to a known clean state.

---

## gpio_set_direction()

Configures the GPIO as:

```text
INPUT or OUTPUT
```

---

## gpio_set_level()

Controls output voltage level.

```text
1 → HIGH
0 → LOW
```

---

# vTaskDelay() and RTOS Scheduling

The firmware used:

```c
vTaskDelay(pdMS_TO_TICKS(1000));
```

instead of Arduino-style:

```c
delay(1000);
```

This is extremely important.

---

# Why vTaskDelay() Matters

During `vTaskDelay()`:

- CPU time is yielded back to scheduler
- other tasks can execute
- networking tasks can run
- WiFi stack can run
- OTA tasks can run

This demonstrates the multitasking nature of ESP-IDF.

---

# Flashing Firmware

Firmware was flashed using:

```bash
idf.py -p /dev/ttyUSB0 flash
```

ESP-IDF:

- resets ESP32
- enters bootloader mode
- flashes bootloader
- flashes partition table
- flashes firmware image

---

# Serial Monitoring

Serial logs were monitored using:

```bash
idf.py -p /dev/ttyUSB0 monitor
```

The monitor displayed:

- bootloader logs
- partition information
- FreeRTOS startup
- firmware logs

---

# Understanding the ESP32 Boot Process

The serial monitor revealed the actual ESP32 boot sequence.

The full boot flow:

```text
ROM Bootloader
    ↓
2nd Stage Bootloader
    ↓
Partition Table Read
    ↓
Firmware Loading
    ↓
FreeRTOS Startup
    ↓
app_main()
```

---

# ROM Bootloader

The ROM bootloader is permanently stored inside ESP32 silicon.

Responsibilities:

- basic hardware initialization
- boot mode detection
- loading second-stage bootloader

This cannot be modified by the user.

---

# Second Stage Bootloader

Generated as:

```text
bootloader.bin
```

Responsibilities:

- flash initialization
- partition table parsing
- OTA slot selection
- firmware loading
- rollback support

This becomes critical for OTA systems.

---

# Partition Tables

One of the most important concepts explored.

Initially, the default partition table contained:

```text
nvs
phy_init
factory
```

This represented a:

```text
single firmware architecture
```

Only one firmware image existed.

This architecture does not support reliable OTA.

---

# Understanding Flash Memory Layout

ESP32 firmware is not simply stored as one continuous application.

Flash memory contains multiple regions:

- bootloader
- partition table
- NVS
- OTA metadata
- firmware slots
- filesystem partitions

This is a fundamental embedded systems concept.

---

# Flash Size Configuration

An important issue was discovered:

```text
Detected size(4096k) larger than image header(2048k)
```

This revealed:

- actual flash = 4MB
- firmware configuration assumed 2MB

This was corrected using:

```bash
idf.py menuconfig
```

under:

```text
Serial flasher config → Flash size → 4MB
```

After rebuilding, the mismatch warning disappeared.

---

# menuconfig

`menuconfig` is one of the most important tools in ESP-IDF.

Opened using:

```bash
idf.py menuconfig
```

This terminal-based interface configures:

- flash settings
- partition settings
- FreeRTOS settings
- logging
- networking
- OTA behavior

ESP-IDF stores these configurations in:

```text
sdkconfig
```

---

# Important Realization

Firmware behavior is heavily:

```text
configuration-driven
```

and not only source-code driven.

---

# OTA Partition Architecture

The partition table was changed to:

```text
Factory app, two OTA definitions
```

This generated:

```text
nvs
otadata
phy_init
factory
ota_0
ota_1
```

---

# Understanding OTA Architecture

This architecture enables:

- A/B firmware updates
- rollback support
- safe firmware switching
- recovery after failed update

---

# otadata Partition

One of the most important discoveries.

`otadata` stores:

- active OTA slot
- update state
- rollback state
- boot information

The bootloader reads `otadata` to decide:

```text
Which firmware partition to boot
```

---

# A/B Firmware Concept

With OTA partitions:

| Partition      | Role             |
| -------------- | ---------------- |
| Running slot   | current firmware |
| Alternate slot | update target    |

Example:

```text
Currently running → ota_0
Download new firmware → ota_1
Reboot
Bootloader switches to ota_1
```

The currently running firmware is never overwritten.

This is the foundation of safe OTA.

---

# Failure Recovery and Rollback

One of the most important OTA concepts learned.

Suppose:

- power failure during update
- corrupted firmware
- firmware crash
- boot failure

The bootloader can:

- detect failure
- revert to previous slot
- restore working firmware

This is possible because:

```text
Old firmware remains intact in alternate partition
```

---

# Why Single Partition OTA Is Dangerous

Without dual partitions:

```text
Firmware must overwrite itself during update
```

If interrupted:

```text
Device may become unusable
```

This is why dual partition OTA architecture is industry standard.

---

# Firmware Size Constraints

Another important realization:

Each OTA partition had a fixed size.

Example:

```text
ota_0 → 1MB
ota_1 → 1MB
```

This means:

```text
Firmware size must fit within OTA slot size
```

This directly affects:

- feature growth
- debugging symbols
- logging
- TLS usage
- libraries included

This is an important embedded systems tradeoff.

---

# Major Engineering Lessons Learned

## 1. OTA Is Primarily a Flash Architecture Problem

OTA is not only:

- HTTP downloads
- firmware transfer

Instead:

```text
OTA fundamentally depends on flash partition architecture
```

---

## 2. ESP-IDF Is RTOS-Centric

Even simple applications already:

- run under FreeRTOS
- use scheduling
- support multitasking

---

## 3. ESP-IDF Is Component-Based

Firmware systems are built using:

```text
modular components
```

rather than monolithic sketches.

---

## 4. Firmware Is Configuration-Driven

Behavior is controlled not only by code, but also by:

- sdkconfig
- partition tables
- bootloader configuration

---

## 5. Bootloaders Are Critical in OTA Systems

The bootloader controls:

- firmware selection
- rollback
- recovery
- partition switching

---

## 6. Embedded Systems Require Memory Planning

Flash memory and RAM are limited.

Partition allocation directly affects:

- OTA reliability
- feature expansion
- filesystem availability
- firmware size limits

---

# Current Project Status

The DFMS project currently has:

- ESP-IDF environment setup
- Stable ESP-IDF v5.5 installation
- Working build system
- Working flashing workflow
- Working serial monitor workflow
- Functional GPIO firmware
- Understanding of FreeRTOS basics
- Understanding of boot process
- Understanding of partition tables
- OTA partition architecture enabled
- A/B firmware architecture configured

---

# Planned Next Learning Phase

The next phase will focus on:

- WiFi initialization
- Event loops
- ESP-IDF logging system
- HTTP client/server
- Networking architecture
- OTA APIs
- Local OTA implementation
- HTTP OTA implementation

These topics build directly on the partition and bootloader concepts already explored.

---

# Final Reflection

A major realization from this exploration was:

```text
Modern embedded firmware systems are much closer to operating-system-driven software architectures than simple microcontroller loops.
```

ESP-IDF exposes:

- RTOS concepts
- modular architectures
- boot management
- flash memory planning
- system initialization
- multitasking

This makes ESP-IDF significantly more suitable for advanced firmware systems like DFMS.

The exploration phase established the foundational understanding required before implementing reliable OTA systems.
