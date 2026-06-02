# 🔧 Bootloader + Application Integration on STM32H7

This project outlines how to configure and integrate a **bootloader** with a **secondary firmware application** on an STM32H7 microcontroller. The bootloader starts at the base of flash (`0x08000000`), while the application resides at a defined offset (e.g., `0x08040000`).

---

## 📐 Application Design

- **Bootloader Start Address**: `0x08000000`
- **Application Start Address**: `0x08040000`
- Both bootloader and application are built separately.

---

## 🚀 Steps to Boot Bootloader + Application Together

### 📍 Step 1: Define Memory Regions in Linker Scripts

Modify the memory layout in the linker scripts of both bootloader and application:

- **Bootloader**: Define flash origin and size starting at `0x08000000` (e.g., 64KB).
- **Application**: Define flash origin and size starting at `0x08040000` (e.g., 512KB).

---

### 📍 Step 2: Set Vector Table Offset in Application

Configure the application's vector table offset to align with its flash origin (`0x08040000`) to ensure proper interrupt handling.

---

### 📍 Step 3: Implement Bootloader Jump to Application

In the bootloader, implement logic to read the application's reset handler from the offset address and jump to it after verifying boot conditions.

---

### 📍 Step 4: Flashing the Firmware

- Flash bootloader and application separately to their respective addresses using STM32CubeProgrammer or other tools to:.
- OR add Application project in startup tab under the Debug Configurations window in STM32Cube IDE.

"C:\Users\janet\OneDrive\Pictures\Screenshots\Screenshot 2025-05-27 082738.png"

---



![Screenshot 2025-05-27 082957](https://github.com/user-attachments/assets/b2750f1d-e9f0-40ab-b086-f837df1c9453)



## 🛠 Tools Used

- STM32CubeIDE
- STM32CubeProgrammer
- STM32H723 development board

---
