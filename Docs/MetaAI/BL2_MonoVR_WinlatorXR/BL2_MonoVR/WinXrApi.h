#pragma once
#include "pch.h"
#include "WinXrApiUDP.h"

class WinXrApi {
public:
    bool Init(); // writes marker files + starts UDP
    void Shutdown();
    bool GetPose(XrPacket& out);
    void SendHaptics(float left, float right, int modeVR, int mode3D, float fovW, float fovH);

    // Current frame id we rendered with - for red pixel
    int GetLastRenderedFrameID() const { return lastFrameID; }
    void SetLastRenderedFrameID(int id) { lastFrameID = id; }

private:
    bool WriteMarkerFiles();

    WinXrApiUDP udp;
    SOCKET sendSock = INVALID_SOCKET;
    sockaddr_in sendAddr{};
    int lastFrameID = 0;
};
