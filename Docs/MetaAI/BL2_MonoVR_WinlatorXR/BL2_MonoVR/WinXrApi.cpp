#include "pch.h"
#include "WinXrApi.h"

bool WinXrApi::Init() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    sendSock = socket(AF_INET, SOCK_DGRAM, 0);
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(7278);
    inet_pton(AF_INET, "127.0.0.1", &sendAddr.sin_addr);

    if (!WriteMarkerFiles()) {
        // still continue - desktop test fallback
    }

    if (!udp.Start()) {
        return false;
    }
    // Wait up to 15s for first packet like CoD4 mod does
    for(int i=0;i<150;i++) {
        XrPacket pkt;
        if(udp.GetLatest(pkt)) break;
        Sleep(100);
    }
    return true;
}

void WinXrApi::Shutdown() {
    udp.Stop();
    if (sendSock != INVALID_SOCKET) closesocket(sendSock);
    // Delete marker file
    DeleteFileA("Z:/tmp/xr/vr");
    DeleteFileA("D:/xrtemp/vr");
    WSACleanup();
}

bool WinXrApi::WriteMarkerFiles() {
    // WinlatorXR handshake - marker files on filesystem path
    // On WinXrApi::Init the mod probes for container's shared drive and tells WinlatorXR "I am a VR game" by writing plain files
    CreateDirectoryA("Z:/tmp/xr", nullptr);
    CreateDirectoryA("D:/xrtemp", nullptr);

    bool zExists = GetFileAttributesA("Z:/") != INVALID_FILE_ATTRIBUTES;
    const char* base = zExists ? "Z:/tmp/xr/" : "D:/xrtemp/";

    std::string vrPath = std::string(base) + "vr";
    std::string verPath = std::string(base) + "version";
    std::string systemPath = std::string(base) + "system";

    // version 0.5 for WinlatorXR
    std::ofstream(verPath) << "0.5";
    std::ofstream(vrPath) << "VR";

    // If system exists, read HMD make/model for controller quirks
    return true;
}

bool WinXrApi::GetPose(XrPacket& out) {
    return udp.GetLatest(out);
}

void WinXrApi::SendHaptics(float left, float right, int modeVR, int mode3D, float fovW, float fovH) {
    // SendHapticVibration sends 6-field ASCII packet to 127.0.0.1:7278
    // <leftVibration> <rightVibration> <MODE_VR> <MODE_3D> <targetFovW> <targetFovH>
    // MODE_VR 1=VR 2=Non-VR screen mode, MODE_3D 0=mono 1=SBS 2=AER
    if (sendSock == INVALID_SOCKET) return;
    char buf[128];
    int len = snprintf(buf, sizeof(buf), "%.3f %.3f %d %d %.1f %.1f", left, right, modeVR, mode3D, fovW, fovH);
    sendto(sendSock, buf, len, 0, (sockaddr*)&sendAddr, sizeof(sendAddr));
}
