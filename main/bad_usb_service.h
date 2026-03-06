#ifndef BAD_USB_SERVICE_H
#define BAD_USB_SERVICE_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <memory>
#include <string>

#include "USBHIDKeyboard.h"
#include "ducky_parser.h"

// Define the type of command being sent to the BadUsb background task
enum class BadUsbCommandType { RUN_SCRIPT, TYPE_TEXT, STOP };

// Message structure for the FreeRTOS Queue
struct BadUsbMessage {
    BadUsbCommandType type;
    char payload[512];  // We accept strings up to 512 bytes for typing or small scripts
};

class BadUsbService {
public:
    BadUsbService();
    ~BadUsbService();

    // Start the background USB processing task and mount TinyUSB
    void Start();

    // Commands intended to be called by the MCP Server
    bool RunScript(const std::string& script);
    bool TypeText(const std::string& text);
    void Stop();

    // Get current status of the service
    std::string GetStatusJSON();

    bool IsRunning() const;

private:
    std::unique_ptr<USBHIDKeyboard> hid_keyboard_;
    std::unique_ptr<DuckyParser> ducky_parser_;

    TaskHandle_t task_handle_;
    QueueHandle_t command_queue_;

    bool is_service_running_;

    // Static FreeRTOS task entry point
    static void UsbTaskWrapper(void* arg);

    // Main processing loop running on Core 0
    void UsbTask();

    // Internal resource isolation hooks (Savaş Modu)
    void EnterCombatMode();
    void ExitCombatMode();
};

#endif  // BAD_USB_SERVICE_H
