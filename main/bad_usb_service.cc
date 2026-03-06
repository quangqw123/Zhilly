#include "bad_usb_service.h"
#include <cstring>
#include <string_view>
#include "application.h"
#include "esp_log.h"

static const char* TAG = "BadUsbService";

BadUsbService::BadUsbService() : task_handle_(nullptr), is_service_running_(false) {
    hid_keyboard_ = std::make_unique<USBHIDKeyboard>();
    ducky_parser_ = std::make_unique<DuckyParser>(hid_keyboard_.get());

    // Create a queue that can hold up to 5 commands
    command_queue_ = xQueueCreate(5, sizeof(BadUsbMessage));
}

BadUsbService::~BadUsbService() {
    if (task_handle_ != nullptr) {
        vTaskDelete(task_handle_);
    }
    if (command_queue_ != nullptr) {
        vQueueDelete(command_queue_);
    }
}

void BadUsbService::Start() {
    if (is_service_running_)
        return;

    ESP_LOGI(TAG, "Starting BadUsbService...");
    hid_keyboard_->begin();  // Initializes TinyUSB internally if needed for this interface
    ESP_LOGI(TAG, "HID Keyboard begin() returned.");

    // Create the background task pinned to Core 0 (background processes)
    xTaskCreatePinnedToCore(UsbTaskWrapper, "BadUsbTask", 4096, this,
                            5,  // Priority
                            &task_handle_,
                            0  // Core 0
    );

    is_service_running_ = true;
    ESP_LOGI(TAG, "BadUsbService Started on Core 0");
}

void BadUsbService::UsbTaskWrapper(void* arg) {
    BadUsbService* service = static_cast<BadUsbService*>(arg);
    service->UsbTask();
}

void BadUsbService::UsbTask() {
    BadUsbMessage msg;
    while (true) {
        // Block until a command arrives in the queue
        if (xQueueReceive(command_queue_, &msg, portMAX_DELAY) == pdPASS) {
            // Apply Resource Isolation Pattern (Savaş Modu)
            EnterCombatMode();

            switch (msg.type) {
                case BadUsbCommandType::RUN_SCRIPT:
                    ESP_LOGI(TAG, "Executing DuckyScript...");
                    ducky_parser_->runScript(msg.payload);
                    break;
                case BadUsbCommandType::TYPE_TEXT:
                    ESP_LOGI(TAG, "Typing text directly...");
                    ducky_parser_->typeText(msg.payload);
                    break;
                case BadUsbCommandType::STOP:
                    ESP_LOGI(TAG, "Stopping BadUSB execution.");
                    ducky_parser_->stop();
                    break;
            }

            // Restore Resources
            ExitCombatMode();
        }
    }
}

void BadUsbService::EnterCombatMode() {
    auto& app = Application::GetInstance();

    // Ses ve TTS kapat
    app.GetAudioService().Stop();

    // IR islerini kapat
    app.GetIrService().StopTvBGone();
    app.GetIrService().StopJammer();

    // CC1101 islerini durdur
    app.GetCc1101Service().StopReplay();
    app.GetCc1101Service().StopJammer();

    ESP_LOGW(TAG,
             "SAVAS MODU AKTIF! Agirliklar atildi (Ses, IR, CC1101), donanim hizlandiriliyor.");
    ESP_LOGI(TAG, "Wi-Fi ve Mikrofon acik tutuluyor, yapay zeka dinlemede kalacak.");
}

void BadUsbService::ExitCombatMode() {
    auto& app = Application::GetInstance();

    // Ses sistemini geri ac
    app.GetAudioService().Start();

    ESP_LOGI(TAG, "SAVAS MODU KAPALI. Normale donuldu.");
}

bool BadUsbService::RunScript(const std::string& script) {
    if (!is_service_running_)
        Start();

    BadUsbMessage msg;
    msg.type = BadUsbCommandType::RUN_SCRIPT;

    // Safely copy payload, truncate if too large
    size_t copy_len = std::min(script.length(), sizeof(msg.payload) - 1);
    std::memcpy(msg.payload, script.c_str(), copy_len);
    msg.payload[copy_len] = '\0';

    return xQueueSend(command_queue_, &msg, 0) == pdPASS;
}

bool BadUsbService::TypeText(const std::string& text) {
    if (!is_service_running_)
        Start();

    BadUsbMessage msg;
    msg.type = BadUsbCommandType::TYPE_TEXT;

    size_t copy_len = std::min(text.length(), sizeof(msg.payload) - 1);
    std::memcpy(msg.payload, text.c_str(), copy_len);
    msg.payload[copy_len] = '\0';

    return xQueueSend(command_queue_, &msg, 0) == pdPASS;
}

void BadUsbService::Stop() {
    if (!is_service_running_)
        return;

    // Send the stop signal to parser first to break loops
    ducky_parser_->stop();

    // We can also send a STOP message to flush the queue
    BadUsbMessage msg;
    msg.type = BadUsbCommandType::STOP;
    xQueueSendToFront(command_queue_, &msg, 0);  // High priority jump
}

std::string BadUsbService::GetStatusJSON() {
    bool ready = hid_keyboard_->isConnected();
    bool writing = ducky_parser_->isRunning();

    char json[128];
    snprintf(json, sizeof(json), "{\"usb_connected\": %s, \"is_typing\": %s}",
             ready ? "true" : "false", writing ? "true" : "false");
    return std::string(json);
}

bool BadUsbService::IsRunning() const { return ducky_parser_->isRunning(); }
