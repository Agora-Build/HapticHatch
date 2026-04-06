# HapticHatch

ESP32-S3 firmware that streams haptic force data between two devices over [Agora RTM](https://docs.agora.io/en/real-time-messaging/overview/product-overview) at **100 Hz**. Each device reads a force sensor, encodes it into a 12-byte packet, and fires it to its peer 100 times per second — while simultaneously playing back the incoming force stream on its local haptic actuator.

```
Device A                              Device B
  sensor ──► RTM TX (100 Hz) ──────► RTM RX ──► haptic
  haptic ◄── RTM RX          ◄────── RTM TX (100 Hz) ◄── sensor
```

## Hardware

| Item | Details |
|------|---------|
| SoC  | ESP32-S3 with 8 MB embedded OPI PSRAM |
| Flash | 8 MB |
| Sensor | Force-sensitive resistor (stub: sine wave) |
| Actuator | Haptic motor / LRA (stub: console log) |

## Prerequisites

- [ESP-IDF v5.2.x](https://docs.espressif.com/projects/esp-idf/en/v5.2.3/esp32s3/get-started/)
- [AOSL](https://github.com/AgoraIO-Community/aosl) source tree checked out at `../aosl` (sibling of this repo)
- [Agora account](https://console.agora.io) with RTM enabled
- [`atem`](https://github.com/Agora-Build/Atem) CLI for token generation

## Repository layout

```
HapticHatch/
├── components/
│   ├── aosl/            # ESP-IDF wrapper for the AOSL source tree
│   ├── haptic/          # Haptic driver (stub: logs intensity)
│   ├── rtsa_transport/  # Agora RTSA SDK transport + link glue
│   └── sensor/          # Sensor driver (stub: sine wave at 100 Hz)
├── main/
│   ├── main.c           # app_main: wires sensor → haptic, starts RTM demo
│   ├── rtm_demo.c       # WiFi init, RTM login, 100 Hz TX task, RX callback
│   └── rtm_demo.h
├── partitions.csv        # 3 MB factory partition (required by SDK size)
└── sdkconfig.defaults    # Target, flash, PSRAM, and credential defaults
```

## Configuration

Copy `sdkconfig.defaults` and fill in your credentials before building.

```ini
# sdkconfig.defaults

CONFIG_AGORA_APP_ID="<your-agora-app-id>"
CONFIG_AGORA_RTM_TOKEN="<rtm-token-for-this-device>"
CONFIG_AGORA_RTM_LOCAL_UID="device_a"   # identity of this device
CONFIG_AGORA_RTM_REMOTE_UID="device_b"  # peer to send to / receive from
CONFIG_DEMO_WIFI_SSID="<your-ssid>"
CONFIG_DEMO_WIFI_PASSWORD="<your-password>"
```

### Generate RTM tokens with `atem`

RTM tokens expire after **3600 s**. Regenerate before each flash session:

```bash
atem token rtm create --user-id device_a   # for the first device
atem token rtm create --user-id device_b   # for the second device
```

Paste each token into the matching `sdkconfig.defaults` before building.

## Building

```bash
# 1. Activate ESP-IDF
source /path/to/esp-idf/export.sh

# 2. Remove stale config (required whenever sdkconfig.defaults changes)
rm -f sdkconfig

# 3. Build
idf.py build
```

## Running two devices

### Flash device A (`/dev/ttyUSB0`)

Set `sdkconfig.defaults`:
```ini
CONFIG_AGORA_RTM_LOCAL_UID="device_a"
CONFIG_AGORA_RTM_REMOTE_UID="device_b"
CONFIG_AGORA_RTM_TOKEN="<device_a token>"
```

```bash
rm sdkconfig && idf.py -p /dev/ttyUSB0 build flash
```

### Flash device B (`/dev/ttyUSB1`)

Update `sdkconfig.defaults`:
```ini
CONFIG_AGORA_RTM_LOCAL_UID="device_b"
CONFIG_AGORA_RTM_REMOTE_UID="device_a"
CONFIG_AGORA_RTM_TOKEN="<device_b token>"
```

```bash
rm sdkconfig && idf.py -p /dev/ttyUSB1 build flash
```

### Monitor both simultaneously

```bash
# terminal 1
idf.py -p /dev/ttyUSB0 monitor

# terminal 2
idf.py -p /dev/ttyUSB1 monitor
```

### Expected output

**device_a** (transmitting to device_b, receiving from device_b):
```
I (5120) rtm_demo: RTM login success, starting TX task
I (5130) rtm_demo: TX seq=1    ts=5080 ms  force=0.57
I (5140) rtm_demo: TX seq=2    ts=5090 ms  force=0.59
...
I (6200) rtm_demo: RX from=device_b seq=1  ts=5150 ms  force=0.42
I (6210) haptic:   play intensity=0.420
```

Messages flow in both directions at **100 Hz** (~60 qps sustained per SDK limit). The `force` field carries the sender's current sensor reading; the receiver plays it on the haptic actuator.

## RTM message format

```c
typedef struct {
    uint32_t seq;           // monotonic counter
    uint32_t timestamp_ms;  // sender uptime in ms
    float    force;         // normalised force [0.0, 1.0]
} __attribute__((packed)) rtm_msg_t;   // 12 bytes
```

`custom_type` is set to `"haptic"` so receivers can filter by message class.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `aosl_malloc N byte failed` on boot | PSRAM not enabled | Verify `CONFIG_SPIRAM=y` in sdkconfig.defaults, do `rm sdkconfig` before rebuild |
| `RTM login failed err=101` | Token expired or wrong UID | Regenerate token with `atem`, `rm sdkconfig`, rebuild |
| `TX result state=2` warnings | Peer not yet online | Normal until both devices are logged in |
| `idf.py: command not found` | IDF env not sourced | `source /path/to/esp-idf/export.sh` |
| Build fails: `AOSL_LOG_ERR undeclared` | Outdated AOSL tree (pre-fix) | Update AOSL: `cd ../aosl && git pull` (fix is in AgoraIO-Community/aosl#3) |
