#include "USBHIDKeyboard.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "tinyusb.h"
#include "tinyusb_console.h"
#include "tusb_cdc_acm.h"

// Define un-defined key macros needed by the cpp code that were part of Arduino's logic previously
#ifndef SHIFT
#define SHIFT 0x80
#endif
#ifndef ALT_GR
#define ALT_GR 0x40
#endif
#ifndef ISO_REPLACEMENT
#define ISO_REPLACEMENT 0x32
#endif
#ifndef ISO_KEY
#define ISO_KEY 0x64
#endif

// Standard Keyboard Report Descriptor
static const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_REPORT_ID_KEYBOARD))};

// Configuration Descriptor
// We define a composite descriptor with both CDC and HID
#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)

static const uint8_t configuration_descriptor[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 3, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // CDC Interface (occupies 2 interfaces)
    // Interface number, string index, EP notification address and size, EP data address (out, in)
    // and size
    TUD_CDC_DESCRIPTOR(0, 4, 0x81, 8, 0x02, 0x82, 64),

    // HID Interface (occupies 1 interface)
    // Interface number, string index, protocol, report descriptor len, EP In address, size &
    // polling interval
    TUD_HID_DESCRIPTOR(2, 5, HID_ITF_PROTOCOL_KEYBOARD, sizeof(hid_report_descriptor), 0x83, 16,
                       10)};

// Device Descriptor
static const tusb_desc_device_t device_descriptor = {.bLength = sizeof(tusb_desc_device_t),
                                                     .bDescriptorType = TUSB_DESC_DEVICE,
                                                     .bcdUSB = 0x0200,
                                                     .bDeviceClass = TUSB_CLASS_MISC,
                                                     .bDeviceSubClass = MISC_SUBCLASS_COMMON,
                                                     .bDeviceProtocol = MISC_PROTOCOL_IAD,
                                                     .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
                                                     .idVendor = 0x303A,
                                                     .idProduct = 0x4005,
                                                     .bcdDevice = 0x0100,
                                                     .iManufacturer = 0x01,
                                                     .iProduct = 0x02,
                                                     .iSerialNumber = 0x03,
                                                     .bNumConfigurations = 0x01};

// String Descriptors
static const char* string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
    "LilyGo",                    // 1: Manufacturer
    "T-Embed CC1101",            // 2: Product
    "123456",                    // 3: Serials
    "T-Embed CDC",               // 4: CDC Interface
    "T-Embed Keyboard",          // 5: HID Interface
};

USBHIDKeyboard::USBHIDKeyboard()
    : _asciimap(KeyboardLayout_en_US), shiftKeyReports(false), _cacheValid(false) {
    memset(&_keyReport, 0, sizeof(KeyReport));
    memset(_keySlotMap, 0, sizeof(_keySlotMap));
    memset(_shiftCache, 0, sizeof(_shiftCache));
    _buildShiftCache();
}

void USBHIDKeyboard::begin(const uint8_t* layout) {
    _asciimap = layout;
    _cacheValid = false;
    _buildShiftCache();

    const tinyusb_config_t tusb_cfg = {
        .port = (tinyusb_port_t)0,
        .phy = {.self_powered = false, .vbus_monitor_io = -1},
        .task = {.size = 4096, .priority = 5, .xCoreID = 0},
        .descriptor =
            {
                .device = &device_descriptor,
                .string = string_desc_arr,
                .string_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]),
                .full_speed_config = configuration_descriptor,
            },
        .event_cb = NULL,
        .event_arg = NULL};

    ESP_LOGI("USBHID", "Installing TinyUSB driver with customized configuration...");
    esp_err_t err = tinyusb_driver_install(&tusb_cfg);
    if (err == ESP_OK) {
        ESP_LOGI("USBHID", "TinyUSB driver installed successfully.");
        esp_err_t console_err = tinyusb_console_init(0);
        if (console_err == ESP_OK) {
            ESP_LOGI("USBHID", "USB Console initialized on CDC interface 0.");
        }
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW("USBHID", "TinyUSB driver already installed.");
    } else {
        ESP_LOGE("USBHID", "TinyUSB driver installation failed: %d", err);
    }
}

// Global callback for HID report descriptor
extern "C" uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return hid_report_descriptor;
}

void USBHIDKeyboard::end() {}

void USBHIDKeyboard::sendReport(KeyReport* keys) {
    if (!tud_hid_ready())
        return;

    uint64_t start_time = esp_timer_get_time();
    while (!tud_hid_ready()) {
        if (esp_timer_get_time() - start_time > 1000 * 100)
            return;  // 100ms timeout
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    tud_hid_keyboard_report(HID_REPORT_ID_KEYBOARD, keys->modifiers, keys->keys);
}

size_t USBHIDKeyboard::pressRaw(uint8_t k) {
    uint8_t i;
    if (k >= 0xE0 && k < 0xE8) {
        // it's a modifier key
        _keyReport.modifiers |= (1 << (k - 0xE0));
    } else if (k && k < 0xA5) {
        int8_t emptySlot = -1;
        for (i = 0; i < 6; i++) {
            if (_keySlotMap[i] == k) {
                return 1;  // Key already pressed
            }
            if (emptySlot == -1 && _keyReport.keys[i] == 0x00) {
                emptySlot = i;  // Remember first empty slot
            }
        }

        if (emptySlot != -1) {
            _keyReport.keys[emptySlot] = k;
            _keySlotMap[emptySlot] = k;
        } else {
            return 0;  // No empty slots
        }
    } else if (_keyReport.modifiers == 0) {
        return 0;
    }
    sendReport(&_keyReport);
    return 1;
}

size_t USBHIDKeyboard::releaseRaw(uint8_t k) {
    uint8_t i;
    if (k >= 0xE0 && k < 0xE8) {
        // it's a modifier key
        _keyReport.modifiers &= ~(1 << (k - 0xE0));
    } else if (k && k < 0xA5) {
        for (i = 0; i < 6; i++) {
            if (_keySlotMap[i] == k) {
                _keyReport.keys[i] = 0x00;
                _keySlotMap[i] = 0x00;
                break;
            }
        }
    }
    sendReport(&_keyReport);
    return 1;
}

size_t USBHIDKeyboard::press(uint8_t k) {
    if (k >= 0x88) {  // it's a non-printing key (not a modifier)
        k = k - 0x88;
    } else if (k >= 0x80) {  // it's a modifier key
        _keyReport.modifiers |= (1 << (k - 0x80));
        k = 0;
    } else {  // it's a printing key (k is a ASCII 0..127)
        if (!_cacheValid) {
            _buildShiftCache();
        }

        k = _shiftCache[k];
        if (!k) {
            return 0;
        }

        if (k & SHIFT) {  // it's a capital letter or other character reached with shift
            if (shiftKeyReports) {
                pressRaw(HID_KEY_SHIFT_LEFT);
            } else {
                _keyReport.modifiers |= 0x02;  // the left shift modifier
            }
            k &= ~SHIFT;
        }
        if (k & ALT_GR) {
            _keyReport.modifiers |= 0x40;  // AltGr = right Alt
            k &= ~ALT_GR;
        }
        if (k == ISO_REPLACEMENT) {
            k = ISO_KEY;
        }
    }
    return pressRaw(k);
}

size_t USBHIDKeyboard::release(uint8_t k) {
    if (k >= 0x88) {  // it's a non-printing key (not a modifier)
        k = k - 0x88;
    } else if (k >= 0x80) {  // it's a modifier key
        _keyReport.modifiers &= ~(1 << (k - 0x80));
        k = 0;
    } else {  // it's a printing key
        if (!_cacheValid) {
            _buildShiftCache();
        }

        k = _shiftCache[k];
        if (!k) {
            return 0;
        }

        if (k & SHIFT) {  // it's a capital letter or other character reached with shift
            if (shiftKeyReports) {
                releaseRaw(k & 0x7F);    // Release key without shift modifier
                k = HID_KEY_SHIFT_LEFT;  // Below, release shift modifier
            } else {
                _keyReport.modifiers &= ~(0x02);  // the left shift modifier
                k &= ~SHIFT;
            }
        }
        if (k & ALT_GR) {
            _keyReport.modifiers &= ~(0x40);  // AltGr = right Alt
            k &= ~ALT_GR;
        }
        if (k == ISO_REPLACEMENT) {
            k = ISO_KEY;
        }
    }
    return releaseRaw(k);
}

void USBHIDKeyboard::releaseAll(void) {
    memset(&_keyReport, 0, sizeof(KeyReport));
    memset(_keySlotMap, 0, sizeof(_keySlotMap));
    sendReport(&_keyReport);
}

size_t USBHIDKeyboard::write(uint8_t c) {
    uint8_t p = press(c);
    vTaskDelay(pdMS_TO_TICKS(this->_delay_ms ? this->_delay_ms : 1));
    release(c);
    vTaskDelay(pdMS_TO_TICKS(this->_delay_ms ? this->_delay_ms : 1));
    return p;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                          hid_report_type_t report_type, uint8_t* buffer,
                                          uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                                      hid_report_type_t report_type, uint8_t const* buffer,
                                      uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

size_t USBHIDKeyboard::write(const uint8_t* buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        if (*buffer != '\r') {
            if (write(*buffer)) {
                n++;
            } else {
                break;
            }
        }
        buffer++;
    }
    return n;
}

void USBHIDKeyboard::_buildShiftCache() {
    for (uint8_t i = 0; i < 128; i++) {
        uint8_t mapValue = _asciimap[i];
        _shiftCache[i] = mapValue;
    }
    _cacheValid = true;
}
