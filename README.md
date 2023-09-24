# SampX

SampX is the custom launcher of SA-MP.

https://www.blast.hk/threads/105888/

## Benefits
* Interface with a dark and light theme.
* Supports multiple languages.
* Can load a list of servers from the master server.
* You can create separate groups for servers.
* Server search in the list.
* System of profiles, which can contain various parameters:
  * Profile name.
  * Nickname.
  * Network adapter, works only with [RiNetworkAdapter](https://github.com/RinatNamazov/samp_network_adapter_rust).
  * Proxy, only works with [RiProxy](https://www.blast.hk/threads/91897/).
  * Game directory.
  * Name of the game's executable file.
  * The version of the sampa.
  * Comment.

![SampX_dark](https://user-images.githubusercontent.com/28570920/141757854-e8a04c00-6b98-4fae-bf7d-92d830762eb9.png)

![SampX_light](https://user-images.githubusercontent.com/28570920/141757864-dd28346e-9cf1-43b4-9771-ca1d73a4a6b4.png)

## Build instructions

#### Required software
* [CMake](https://cmake.org)
* [Visual Studio](https://visualstudio.microsoft.com)
* [Vcpkg](https://github.com/microsoft/vcpkg)

#### Building
```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```

## Third-party

* Qt ([LGPL](http://doc.qt.io/qt-5/lgpl.html))
* CMake ([New BSD License](https://github.com/Kitware/CMake/blob/master/Copyright.txt))
* QDarkStyleSheet ([MIT License](https://github.com/ColinDuquesnoy/QDarkStyleSheet/blob/master/LICENSE.rst))
* pe-parse ([MIT License](https://github.com/trailofbits/pe-parse/blob/master/LICENSE))

## License
The source code is published under GPLv3, the license is available [here](LICENSE).
