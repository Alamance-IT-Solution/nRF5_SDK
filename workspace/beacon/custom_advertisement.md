Setting `USE_UICR_FOR_MAJ_MIN_VALUES` involves writing values to the User Information Configuration Register (UICR) of the nRF52 series device. The UICR is a region of memory where certain parameters and configuration settings can be stored.

To set `USE_UICR_FOR_MAJ_MIN_VALUES`, you need to follow these steps:

1. **Install nrfjprog:**
   Make sure you have the JLinkARM from Segger and Nordic nrfjprog tool installed. You can download it from the Nordic Semiconductor website.
   
2. **Connect the Dongle:**
   Ensure that your nRF52 Dongle is connected to your computer and press reset.

3. **Find Serial Number:**
   Identify the serial number of your nRF52 Dongle. You can find it on the dongle or by using the following command:

   ```bash
   nrfjprog -i
   ```
   Alternatively find the serial number in the programming tool from NRF Connect. In this case its: `CD21B872F31E`
   
4. **Write to UICR:**
   Use the following command to write the major and minor values to the UICR:
   
   ```bash
   nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val <your major/minor value>
   ```

   Replace `<Segger-chip-Serial-Number>` with the actual serial number of your dongle. Replace `<your major/minor value>` with the desired major and minor values encoded in hexadecimal.

   For example, to set a major value of `0xabcd` and a minor value of `0x0102`, you would run:

   ```bash
   nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val 0xabcd0102
   ```

   Note: Ensure that you're using the correct serial number for your dongle.

   I ran:
   ```bash
   $ nrfjprog --snr CD21B872F31E --memwr 0x10001080 --val 0xabcd0102 --log
   ERROR: Argument provided has a wrong value. Memory access not aligned, value
   ERROR: outside of range or similar.
   ```
   
   See: https://devzone.nordicsemi.com/f/nordic-q-a/19912/nrfjprog-error-the-system-cant-find-the-file-specified
   
5. **Verify Settings:**
   You can verify that the values were written successfully by reading the UICR memory. Use the following command:

   ```bash
   nrfjprog --snr <Segger-chip-Serial-Number> --memrd 0x10001080
   ```

   It should display the values you wrote.

After these steps, when you compile and run your application with `USE_UICR_FOR_MAJ_MIN_VALUES` defined, it should use the major and minor values from the UICR. Keep in mind that you only need to set these values once, and they will persist across resets of the device. If you want to change the values, you can repeat the steps with the new values.


To compile the application with `USE_UICR_FOR_MAJ_MIN_VALUES`, you need to make sure that the macro is defined in your source code or build system. In your provided source code, there's already a section mentioning `USE_UICR_FOR_MAJ_MIN_VALUES`:

```c
#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
// If USE_UICR_FOR_MAJ_MIN_VALUES is defined, the major and minor values will be read from the
// UICR instead of using the default values.
// ...
#endif
```

If you want to use this feature, make sure that `USE_UICR_FOR_MAJ_MIN_VALUES` is defined. You can define it directly in your source code or through your build system (e.g., Makefile, CMakeLists.txt).

### Method 1: Define in Source Code

You can add the following line at the beginning of your source code, before the `#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)` section:

```c
#define USE_UICR_FOR_MAJ_MIN_VALUES
```

This line explicitly defines the macro, enabling the code within the corresponding `#if` block.

### Method 2: Define in Build System

1. **Makefile:**
   If you are using a Makefile for building your project, you can add the following compiler flag:

   ```make
   CFLAGS += -DUSE_UICR_FOR_MAJ_MIN_VALUES
   ```

   This flag tells the compiler to define `USE_UICR_FOR_MAJ_MIN_VALUES`.

2. **CMake:**
   If you are using CMake, you can add the following line to your `CMakeLists.txt`:

   ```cmake
   target_compile_definitions(your_target_name PRIVATE USE_UICR_FOR_MAJ_MIN_VALUES)
   ```

   Replace `your_target_name` with the actual name of your target.

Once you have defined `USE_UICR_FOR_MAJ_MIN_VALUES`, recompile your application. The major and minor values should now be read from the UICR as specified in the source code.