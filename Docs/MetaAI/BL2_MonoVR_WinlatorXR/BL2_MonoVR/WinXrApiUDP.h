#pragma once
#include "pch.h"
#include "WinXrApi.h"

// WinlatorXR UDP wire format - 28 floats + FrameID + 21 bools
// L quat(4) L stick(2) L pos(3) R quat(4) R stick(2) R pos(3) HMD quat(4) HMD pos(3) IPD FOV_H FOV_V
struct XrPacket {
    float L_quat[4] = {0,0,0,1};
    float L_stick[2] = {0,0};
    float L_pos[3] = {0,0,0};
    float R_quat[4] = {0,0,0,1};
    float R_stick[2] = {0,0};
    float R_pos[3] = {0,0};
    float HMD_quat[4] = {0,0,0,1};
    float HMD_pos[3] = {0,0,0};
    float IPD = 0.063f;
    float FOV_H = 90.f;
    float FOV_V = 90.f;
    int FrameID = 0;
    bool buttons[21] = {false}; // L_GRIP, L_MENU, L_STICK_PRESS, L_STICK_L/R/U/D, L_TRIGGER, L_X, L_Y, R_A, R_B, R_GRIP, R_STICK_PRESS, R_STICK_L/R/U/D, R_TRIGGER, IS_IMMERSIVE, IS_SBS

    // Converted poses in UE3 space (after scale)
    FVector HMD_pos_ue;
    FQuat HMD_quat_ue;
    FVector R_pos_ue;
    FQuat R_quat_ue;
    FVector L_pos_ue;
    FQuat L_quat_ue;
};

class WinXrApiUDP {
public:
    bool Start();
    void Stop();
    bool GetLatest(XrPacket& out);

private:
    void RecvThread();
    bool Parse(const char* buf, int len, XrPacket& out);

    SOCKET sock = INVALID_SOCKET;
    std::thread thread;
    std::atomic<bool> running{false};
    std::mutex mtx;
    XrPacket latest;
    bool hasData = false;
};
