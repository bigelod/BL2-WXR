#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sstream>
#include <cmath>

// UE3 basic types - replace with real SDK when you have it
struct FVector { float X, Y, Z; };
struct FQuat { float X, Y, Z, W; };
struct FRotator { int Pitch, Yaw, Roll; };

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "d3d9.lib")
