# VL53L_MZ ESPHome Component

Custom ESPHome component for STMicroelectronics multizone Time-of-Flight (ToF) ranging sensors.

Supports the following STMicroelectronics multizone Time-of-Flight (ToF) sensors:

- **VL53L5CX** — 65° diagonal FoV, up to 4 m range
- **VL53L7CX** — 90° diagonal FoV, up to 3.5 m range
- **VL53L8CX** — 65° diagonal FoV, up to 4 m range, 2nd generation, lower power than VL53L5CX

> **⚠️ Disclaimer:** This software is **experimental**. It has only been tested on the **ESP32-C6** using the **ESP-IDF**
> framework. The following features are **untested**:
> - VL53L7CX support
> - VL53L8CX support
> - Multiple sensors on the same I²C bus
> - Crosstalk (Xtalk) calibration (both running calibration and applying pre-calibrated data)

## Features

- **Multizone ranging**: 4×4 (16 zones) or 8×8 (64 zones) resolution
- **Two ranging modes**: Continuous (high performance) and Autonomous (low power)
- **Configurable integration time**, sharpening, and target order
- **Crosstalk (Xtalk) calibration**: Run calibration or apply pre-calibrated data
- **Multiple sensors**: Support for multiple ToF sensors on the same I²C bus (via LP pin)
- **Numeric sensors**: Per-zone distance, sigma, reflectance, and target count with aggregation modes (average, nearest,
  farthest, center)
- **Text sensors**: CSV, table, binary, and PNG output of all zone data with axis flipping support

## Requirements

- MCU variant with sufficient RAM and Flash (e.g., ESP32-C6)
- I²C bus configured at up to 1000 kHz

## Installation

Add this component to your ESPHome project using an external component source. For a local installation, place the
`components/vl53l_mz/` next to your config file and reference it:

```yaml
external_components:
  - source:
      type: local
      path: components
```

For a remote installation from GitHub:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/SG-O/VL53L_MZ_esphome
```

## Example

A complete working example configuration is available in [`example.yaml`](example.yaml).

## Configuration

### VL53L MZ Hub (`vl53l_mz`)

The main component configures the sensor hardware and ranging parameters.

```yaml
vl53l_mz:
  id: sensor_01
  model: VL53L5CX
  address: 0x29
  i2c_id: bus_a
  reset_pin: GPIO2
  interrupt_pin: GPIO3
  lp_pin: GPIO18
  resolution: 8X8
  ranging_frequency: 1Hz
  ranging_mode: AUTO
  target_order: STRONGEST
  integration_time: 5ms
  sharpening: 14%
  continuous_update: false
  update_interval: 60s
```

#### Configuration Variables

| Key                                 | Type       | Default     | Description                                                                                                                                                                                                                                              |
|-------------------------------------|------------|-------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **`id`**                            | ID         | -           | Unique ID for this sensor hub.                                                                                                                                                                                                                           |
| **`model`**                         | enum       | -           | **Required.** Sensor model. See **Sensor Models** below.                                                                                                                                                                                                 |
| **`address`**                       | int        | `0x29`      | I²C address (7-bit). Default is `0x29`. If changed, `lp_pin` must be defined.                                                                                                                                                                            |
| **`i2c_id`**                        | ID         | -           | ID of the I²C bus component.                                                                                                                                                                                                                             |
| **`reset_pin`**                     | GPIO pin   | -           | Reset output pin (active high). Used to reset the sensor's I²C buffer. Does not reset the sensor itself. Usually not needed.                                                                                                                             |
| **`interrupt_pin`**                 | GPIO pin   | -           | Interrupt input pin (active low). The sensor drives this pin low when new data is available.                                                                                                                                                             |
| **`lp_pin`**                        | GPIO pin   | -           | LPn (low power) output pin. **Required** when using an I²C address other than `0x29`.                                                                                                                                                                    |
| **`resolution`**                    | enum       | `8X8`       | Zone resolution: `4X4` (16 zones) or `8X8` (64 zones).                                                                                                                                                                                                   |
| **`ranging_frequency`**             | frequency  | `1Hz`       | Ranging frequency. Max 60 Hz for 4×4, 15 Hz for 8×8.                                                                                                                                                                                                     |
| **`ranging_mode`**                  | enum       | `AUTO`      | Ranging mode: `CONTINUOUS` (high performance) or `AUTO` (autonomous, low power).                                                                                                                                                                         |
| **`integration_time`**              | time       | `5ms`       | VCSEL integration time (autonomous mode only). Range: 2 ms – 249 ms.                                                                                                                                                                                     |
| **`sharpening`**                    | percentage | `14%`       | Sharpener percentage (0–99%). See [ST UM2884](https://www.st.com/resource/en/user_manual/um2884-a-guide-to-using-the-vl53l5cx-multizone-timeofflight-ranging-sensor-with-a-wide-field-of-view-ultra-lite-driver-uld-stmicroelectronics.pdf) for details. |
| **`target_order`**                  | enum       | `STRONGEST` | Target reporting order: `CLOSEST` or `STRONGEST`.                                                                                                                                                                                                        |
| **`continuous_update`**             | bool       | `false`     | When `true`, the sensor continuously publishes new data as it becomes available instead of waiting for the update interval.                                                                                                                              |
| **`update_interval`**               | time       | `60s`       | Polling interval. Must be ≥ 1 s. Use `continuous_update: true` for faster updates.                                                                                                                                                                       |
| **`run_xtalk_calibration`**         | bool       | `false`     | Run crosstalk calibration on startup. Cannot be used with `xtalk_calibration_data`.                                                                                                                                                                      |
| **`xtalk_calibration_reflectance`** | percentage | `3%`        | Target reflectance for Xtalk calibration (1–99%).                                                                                                                                                                                                        |
| **`xtalk_calibration_distance`**    | distance   | `600mm`     | Target distance for Xtalk calibration (0.6 m – 3.0 m).                                                                                                                                                                                                   |
| **`xtalk_calibration_data`**        | string     | -           | Base64 encoded pre-calibrated Xtalk data string. Cannot be used with `run_xtalk_calibration`.                                                                                                                                                            |

> **Note:** This component supports multiple sensors on the same I²C bus. When using multiple sensors with different
> addresses, each requires the definition of the `lp_pin` to control the address programming sequence.

#### Sensor Models

| Model      | FoV                    | Max Range |
|------------|------------------------|-----------|
| `VL53L5CX` | 65° diagonal (45°×45°) | 4 m       |
| `VL53L7CX` | 90° diagonal (60°×60°) | 3.5 m     |
| `VL53L8CX` | 65° diagonal (45°×45°) | 4 m       |

### Numeric Sensor (`sensor`)

Creates numeric sensors that publish individual zone values or aggregated values from the sensor zones.

```yaml
sensor:
  - platform: vl53l_mz
    vl53l_mz_id: sensor_01
    name: "Single Zone Distance"
    zone_mode: SINGLE
    zone_data: DISTANCE
    selected_zone: 4
```

#### Configuration Variables

| Key                 | Type | Default    | Description                                                  |
|---------------------|------|------------|--------------------------------------------------------------|
| **`vl53l_mz_id`**   | ID   | -          | ID of the parent `vl53l_mz` hub.                             |
| **`zone_mode`**     | enum | `SINGLE`   | How to aggregate zone data. See **Zone Modes** below.        |
| **`zone_data`**     | enum | `DISTANCE` | Which data to report. See **Zone Data** below.               |
| **`selected_zone`** | int  | `0`        | Zone index to use when `zone_mode` is `SINGLE`. Range: 0–63. |

#### Zone Modes

| Mode       | Description                                                  |
|------------|--------------------------------------------------------------|
| `SINGLE`   | Report data from a single zone specified by `selected_zone`. |
| `AVERAGE`  | Report the average value across all zones.                   |
| `NEAREST`  | Report the value from the zone with the closest target.      |
| `FARTHEST` | Report the value from the zone with the farthest target.     |
| `CENTER`   | Report the value from the center 2x2 zones.                  |

#### Zone Data

| Data           | Unit | Description                                                            |
|----------------|------|------------------------------------------------------------------------|
| `DISTANCE`     | m    | Target distance in meters (3 decimal places).                          |
| `SIGMA`        | m    | Sigma estimator for noise in the reported distance (3 decimal places). |
| `REFLECTANCE`  | %    | Estimated target reflectance in percent (0 decimal places).            |
| `TARGET_COUNT` | -    | Number of detected targets in the zone.                                |

### Text Sensor (`text_sensor`)

Creates text sensors that output all zone data in various formats, useful for visualization or bulk data transfer.

```yaml
text_sensor:
  - platform: vl53l_mz
    vl53l_mz_id: sensor_01
    name: "Distance CSV"
    output_formatting: CSV
    output_data: FLOAT_DISTANCE
    flip_x: false
    flip_y: false
```

#### Configuration Variables

| Key                     | Type | Default          | Description                                      |
|-------------------------|------|------------------|--------------------------------------------------|
| **`vl53l_mz_id`**       | ID   | -                | ID of the parent `vl53l_mz` hub.                 |
| **`output_formatting`** | enum | `CSV`            | Output format. See **Output Formats** below.     |
| **`output_data`**       | enum | `FLOAT_DISTANCE` | Which data to output. See **Output Data** below. |
| **`flip_x`**            | bool | `false`          | Flip the output horizontally.                    |
| **`flip_y`**            | bool | `false`          | Flip the output vertically.                      |

#### Output Formats

| Format   | Description                                                                                                                           |
|----------|---------------------------------------------------------------------------------------------------------------------------------------|
| `CSV`    | Comma-separated values, one row per sensor row.                                                                                       |
| `TABLE`  | Formatted ASCII table.                                                                                                                |
| `BINARY` | Base64 encoded raw binary data (Bytes, Shorts or Floats depending on the selected output data) suitable for programmatic consumption. |
| `PNG`    | Base64 encoded PNG image (8 or 16 bit resolution depending on the selected output data).                                              |

#### Output Data

| Data             | Description                                                                                                                                                                                                                                                                                                    |
|------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `RAW_DISTANCE`   | Raw distance in millimeters (uint16). Invalid targets report 0.                                                                                                                                                                                                                                                |
| `FLOAT_DISTANCE` | Distance in meters (float, 3 decimal places). Invalid targets report `NaN`.                                                                                                                                                                                                                                    |
| `RAW_SIGMA`      | Raw sigma (noise estimator) in millimeters (uint16).                                                                                                                                                                                                                                                           |
| `FLOAT_SIGMA`    | Sigma in meters (float, 3 decimal places).                                                                                                                                                                                                                                                                     |
| `REFLECTANCE`    | Estimated target reflectance in percent (uint8).                                                                                                                                                                                                                                                               |
| `TARGET_COUNT`   | Number of detected targets per zone (uint8).                                                                                                                                                                                                                                                                   |
| `STATUS`         | Target status per zone (uint8). Values 5, 6, and 9 indicate valid measurements. For more details see [ST UM2884](https://www.st.com/resource/en/user_manual/um2884-a-guide-to-using-the-vl53l5cx-multizone-timeofflight-ranging-sensor-with-a-wide-field-of-view-ultra-lite-driver-uld-stmicroelectronics.pdf) |

> **Note on binary formatting:** `RAW_DISTANCE` and
`RAW_SIGMA` use 2 bytes (uint16) per zone; `FLOAT_DISTANCE` and `FLOAT_SIGMA` use 4 bytes (float) per zone;
`REFLECTANCE`, `TARGET_COUNT`, and `STATUS` use 1 byte (uint8) per zone.

> **Note on PNG formatting:** `RAW_DISTANCE` and
`RAW_SIGMA` use 16-bit pixels per zone; `FLOAT_DISTANCE` and
`FLOAT_SIGMA` are are multiplied by 16000 and encoded as 16-bit pixels (to fully use the dynamic range); `REFLECTANCE`,
`TARGET_COUNT`, and `STATUS` use 8-bit pixels.

## Example

## Multiple Sensors

This component supports multiple sensors on the same I²C bus. Since sensors of the same type share the same default I²C address,
you must connect the LP pin to a free output of your MCU and specify the `lp_pin` option.

The component handles the address programming sequence automatically: it first sets all LP pins low, then
brings them high one by one to bring each sensor online, and program the new address.

```yaml
vl53l_mz:
  - id: sensor_01
    model: VL53L5CX
    address: 0x29
    i2c_id: bus_a
    lp_pin: GPIO1
    resolution: 4X4
    ranging_frequency: 30Hz
  - id: sensor_02
    model: VL53L5CX
    address: 0x2A
    i2c_id: bus_a
    lp_pin: GPIO2
    resolution: 4X4
    ranging_frequency: 30Hz
```

> **Important:** When using multiple sensors, every one (including the one at the default address) must define a
`lp_pin`.

> **Note:** Different sensor models (e.g., VL53L5CX, VL53L7CX, VL53L8CX) can be mixed on the same I²C bus.

## Zone Layout

The sensor divides its field of view into zones. Zone indices are numbered row by row, starting from the
top-left from the sensor's perspective:

### 4×4 Resolution (16 zones)

```
  0  1  2  3 <
  4  5  6  7
  8  9 10 11
 12 13 14 15
```

### 8×8 Resolution (64 zones)

```
  0  1  2  3  4  5  6  7 <
  8  9 10 11 12 13 14 15
 16 17 18 19 20 21 22 23
 24 25 26 27 28 29 30 31
 32 33 34 35 36 37 38 39
 40 41 42 43 44 45 46 47
 48 49 50 51 52 53 54 55
 56 57 58 59 60 61 62 63
```

> **Note:** `<` Indicates the location of the marking on the sensor's package.

## License

This component is licensed under the BSD 3-Clause License. See [LICENSE](LICENSE) for details.

The uncompng library created by the Wuffs Authors that is included in this component is provided under its own license.
See [uncompng.c](components/vl53l_mz/text_sensor/uncompng.c) for details.
