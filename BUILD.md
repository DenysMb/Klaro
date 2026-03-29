## Build

### Prerequisites
- CMake 3.20 or higher
- Qt6 (Core, Quick, Gui, QuickControls2, Widgets)
- KDE Frameworks 6 (Kirigami, I18n, CoreAddons, QQC2DesktopStyle, IconThemes)
- C++17 compatible compiler
- Git

### Build Instructions

1. **Clone the repository:**
```bash
git clone https://invent.kde.org/denysmb/Klaro.git
cd Klaro
```

2. **Configure the build:**
```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=~/.local
```

3. **Build:**
```bash
cmake --build build
```

4. **Install:**
```bash
cmake --install build
```

5. **Update desktop database and icon cache:**
```bash
update-desktop-database ~/.local/share/applications/
gtk-update-icon-cache ~/.local/share/icons/hicolor/
```

6. **Run:**
```bash
~/.local/bin/klaro
```

Or search for "Klaro" in your application launcher.

### System-wide Installation

For system-wide installation, omit the `CMAKE_INSTALL_PREFIX` and use sudo for install:

```bash
cmake -B build
cmake --build build
sudo cmake --install build
sudo update-desktop-database
sudo gtk-update-icon-cache /usr/share/icons/hicolor/
```
