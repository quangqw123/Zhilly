#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <vector>

// Forward declaration of the HID interface
class HIDInterface;

// Command Types for parsing
enum DuckyCommandType {
    DuckyCommandType_Cmd,
    DuckyCommandType_Print,
    DuckyCommandType_Delay,
    DuckyCommandType_Comment,
    DuckyCommandType_Repeat,
    DuckyCommandType_Combination,
    DuckyCommandType_WaitForButtonPress,  // (Probably ignored in our AI context or handled
                                          // specially)
    DuckyCommandType_AltChar,
    DuckyCommandType_AltString,
    DuckyCommandType_StringDelay,
    DuckyCommandType_DefaultStringDelay
};

struct DuckyCommand {
    const char* command;
    char key;
    DuckyCommandType type;
};

struct DuckyCombination {
    const char* command;
    char key1;
    char key2;
    char key3;
};

class DuckyParser {
private:
    HIDInterface* hid;
    bool is_running;
    bool should_stop;
    uint32_t default_delay;
    uint32_t string_delay;

    // Parses a single line of duckyscript and executes it
    void parseLine(const std::string& line);

    // Helpers
    const DuckyCommand* findDuckyCommand(const char* cmd);
    const DuckyCombination* findDuckyCombination(const char* cmd);
    void sendAltChar(uint8_t charCode);
    void sendAltString(const std::string& text);

    // Core function to execute the split logic (Zero RAM-allocation string parts)
    void executeCommand(const std::string& cmd, const std::string& args);

public:
    DuckyParser(HIDInterface* hid_interface);
    ~DuckyParser();

    // The main entry point to run a multiline DuckyScript from memory
    void runScript(const std::string& script);

    // Provide a way to immediately stop specific long-running delays/scripts
    void stop();

    bool isRunning() const { return is_running; }

    // Fast path for when the AI just wants to type something without a script
    void typeText(const std::string& text);
};
