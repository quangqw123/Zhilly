#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct IrCode {
    std::string name;
    std::string type;
    std::string protocol;
    uint32_t address = 0;
    uint32_t command = 0;
    uint32_t frequency = 38000;
    uint8_t bits = 32;
    std::vector<uint16_t> raw_data;
};

enum class IrJamMode { BASIC, SWEEP, RANDOM };

class IrService {
public:
    IrService();
    ~IrService();

    bool Init();
    void Deinit();

    bool ReplayFile(const std::string& filepath, const std::string& command_name = "");

    bool StartTvBGone(const std::string& region);
    void StopTvBGone();
    bool IsTvBGoneRunning() const { return is_tvbgone_; }

    bool StartJammer(IrJamMode mode = IrJamMode::SWEEP, uint32_t duration_ms = 0);
    void StopJammer();
    bool IsJamming() const { return is_jamming_; }

    bool SendCode(const IrCode& code);

    void _SetTvBGone(bool v) { is_tvbgone_ = v; }
    void _SetJamming(bool v) { is_jamming_ = v; }
    bool _SendRawPublic(const std::vector<uint16_t>& d, uint32_t freq) { return SendRaw(d, freq); }

private:
    bool SendRaw(const std::vector<uint16_t>& durations, uint32_t freq_hz = 38000);
    bool SendNEC(uint32_t address, uint32_t command);
    bool SendRC5(uint32_t address, uint32_t command);
    bool SendRC6(uint32_t address, uint32_t command);
    bool SendSamsung(uint32_t address, uint32_t command);
    bool SendSony(uint32_t address, uint32_t command, uint8_t bits);

    std::vector<IrCode> ParseIrFile(const std::string& filepath);

    rmt_channel_handle_t tx_channel_ = nullptr;
    rmt_encoder_handle_t tx_encoder_ = nullptr;

    bool initialized_ = false;
    bool is_jamming_ = false;
    bool is_tvbgone_ = false;
    void* jammer_task_handle_ = nullptr;
    void* tvbgone_task_handle_ = nullptr;

    static constexpr int IR_TX_GPIO = 2;
    static constexpr int IR_RX_GPIO = 1;
    static constexpr int RMT_CLK_RES_HZ = 1000000;
};
