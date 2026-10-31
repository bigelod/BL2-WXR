#include "pch.h"
#include "WinXrApiUDP.h"

bool WinXrApiUDP::Start() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(7872);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        return false;
    }
    // non-blocking with timeout
    DWORD timeout = 20;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    running = true;
    thread = std::thread(&WinXrApiUDP::RecvThread, this);
    return true;
}

void WinXrApiUDP::Stop() {
    running = false;
    if (thread.joinable()) thread.join();
    if (sock != INVALID_SOCKET) closesocket(sock);
    WSACleanup();
}

bool WinXrApiUDP::GetLatest(XrPacket& out) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!hasData) return false;
    out = latest;
    return true;
}

void WinXrApiUDP::RecvThread() {
    char buf[2048];
    while (running) {
        int len = recv(sock, buf, sizeof(buf)-1, 0);
        if (len > 0) {
            buf[len] = 0;
            XrPacket pkt;
            if (Parse(buf, len, pkt)) {
                std::lock_guard<std::mutex> lock(mtx);
                latest = pkt;
                hasData = true;
            }
        }
    }
}

bool WinXrApiUDP::Parse(const char* buf, int len, XrPacket& out) {
    // Wire is ASCII space separated, human readable - easy to repro from another tool
    // Real captured packet example in HaloCEWXR WinXrApi.cpp:549
    // Format: L quat QX QY QZ QW ... HMD pos ... IPD FOV_H FOV_V <FrameID int> <21 x T/F> <HMD name string ignored>
    std::istringstream iss(std::string(buf, len));
    iss.imbue(std::locale::classic()); // mandatory: container sends '.' decimals

    // Try simple float parse first - WinlatorXR sends lines like: "0.1 0.2 0.3 ..."
    // We parse in order: 28 floats
    float f[28];
    for (int i=0;i<28;i++) {
        if(!(iss >> f[i])) return false;
    }
    out.L_quat[0]=f[0]; out.L_quat[1]=f[1]; out.L_quat[2]=f[2]; out.L_quat[3]=f[3];
    out.L_stick[0]=f[4]; out.L_stick[1]=f[5];
    out.L_pos[0]=f[6]; out.L_pos[1]=f[7]; out.L_pos[2]=f[8];
    out.R_quat[0]=f[9]; out.R_quat[1]=f[10]; out.R_quat[2]=f[11]; out.R_quat[3]=f[12];
    out.R_stick[0]=f[13]; out.R_stick[1]=f[14];
    out.R_pos[0]=f[15]; out.R_pos[1]=f[16]; out.R_pos[2]=f[17];
    out.HMD_quat[0]=f[18]; out.HMD_quat[1]=f[19]; out.HMD_quat[2]=f[20]; out.HMD_quat[3]=f[21];
    out.HMD_pos[0]=f[22]; out.HMD_pos[1]=f[23]; out.HMD_pos[2]=f[24];
    out.IPD=f[25]; out.FOV_H=f[26]; out.FOV_V=f[27];

    int frameId=0;
    if(!(iss >> frameId)) frameId = (frameId+1) & 0xFF;
    out.FrameID = frameId;

    // 21 bools as T/F chars
    for(int i=0;i<21;i++) {
        char c; iss >> c;
        if(c=='T' || c=='t') out.buttons[i]=true;
        else if(c=='F' || c=='f') out.buttons[i]=false;
        else { // no bools - assume false
            out.buttons[i]=false;
        }
    }

    // Convert to UE3: Scale 1m = 50 UU, flip Z for UE3 Z-up
    // OpenXR: +Y up, -Z forward. UE3: Z up, X forward, Y right. Simple conversion for prototype.
    const float UU_PER_M = 50.f;
    out.HMD_pos_ue = { out.HMD_pos[2]*UU_PER_M, out.HMD_pos[0]*UU_PER_M, out.HMD_pos[1]*UU_PER_M };
    out.R_pos_ue = { out.R_pos[2]*UU_PER_M, out.R_pos[0]*UU_PER_M, out.R_pos[1]*UU_PER_M };
    out.L_pos_ue = { out.L_pos[2]*UU_PER_M, out.L_pos[0]*UU_PER_M, out.L_pos[1]*UU_PER_M };
    
    // 180 deg roll fix - XrAPI's controller rotation is rolled 180 deg compared to OpenXR aim pose
    // Apply extra 180 around forward for Quest
    out.HMD_quat_ue = { out.HMD_quat[0], out.HMD_quat[1], out.HMD_quat[2], out.HMD_quat[3] };
    out.R_quat_ue = { out.R_quat[0], out.R_quat[1], out.R_quat[2], out.R_quat[3] };
    out.L_quat_ue = { out.L_quat[0], out.L_quat[1], out.L_quat[2], out.L_quat[3] };

    return true;
}
