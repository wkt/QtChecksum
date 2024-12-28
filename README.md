# How to build ?

## 1 Download [Qt Online Installer](https://download.qt.io/official_releases/online_installers/)
## 2 Install Qt(5.15+) and QtCreator
## 3 Launch QtCreator and open QtChecksum.pro
## 4 Configure build
## 5 Ctrl + R to run the Application

<br>

# How to make package

## Windows
### 1 Change paths in windows/win_release.bat
### 2 run windows/win_release.bat

<br>

## Ubuntu
```
$ sudo apt-get install -y debhelper-compat pkg-config build-essential libnautilus-extension-dev qtbase5-dev qtbase5-dev-tools libqt5svg5-dev

$ dpkg-buildpackage -b
```
<br>

## macOS
### 1 xcode-select --install
### 2 change QT_DIR in macOS/build.sh
### 3 bash macOS/build.sh '1.0.3'
### 

<br>

# Downloads
[Address]()