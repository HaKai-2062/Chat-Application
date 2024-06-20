# Chat Application
![I1](https://raw.githubusercontent.com/HaKai-2062/Chat-Application/master/res/github/ChatApp.png)
### Features
- A room where multiple clients can connect and exchange messages.
- Colored messages can be sent by prefixing them using ^ and 0-9 number e.g. ^1Hello will print Hello in red color.
- Chat history is saved on server side and clients can fetch the chat from server on connect event.
- Server can be run on both Windows and Linux.

## Libraries
- [Asio](https://think-async.com/Asio/)
- [GLFW](https://github.com/glfw/glfw)
- [imgui (docking branch)](https://github.com/ocornut/imgui/tree/docking)
- [OneLoneCoder's Networking Library](https://github.com/OneLoneCoder/Javidx9/blob/ea35889f91735542957ddcecf41761c27db0d8c2/PixelGameEngine/BiggerProjects/Networking/Parts3%264/olcPGEX_Network.h)

## Tools Used
- [CMake](https://cmake.org/)
- [Visual Studio](https://visualstudio.microsoft.com/)

## Building the project
- Open the project directory inside Visual Studio.
- Open the CMakelists.txt file located in the project folder and Ctrl+S. This will clone and build the dependencies.
- In case of CMake related issues, delete 'out' folder and try rebuilding.