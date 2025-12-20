#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// ---------- CRC-16 over bit vector (same as sender) ----------
uint16_t crc16(const vector<int> &bits) {
    uint16_t crc = 0x0000;
    const uint16_t poly = 0x1021;

    for (int bit : bits) {
        bit &= 1;
        int msb = (crc >> 15) & 1;
        crc = (uint16_t)((crc << 1) & 0xFFFF);
        if (msb ^ bit) crc ^= poly;
    }
    return crc;
}

// ---------- Manchester decode: 10 -> 0, 01 -> 1 ----------
bool manchester_decode(const vector<int> &enc, vector<int> &decoded) {
    decoded.clear();

    if (enc.size() % 2 != 0) {
        cerr << "Receiver: Encoded bit length is odd => invalid Manchester.\n";
        return false;
    }

    decoded.reserve(enc.size() / 2);

    for (size_t i = 0; i < enc.size(); i += 2) {
        int a = enc[i];
        int b = enc[i + 1];

        if (a == 1 && b == 0) decoded.push_back(0);
        else if (a == 0 && b == 1) decoded.push_back(1);
        else {
            cerr << "Receiver: Invalid Manchester pair (" << a << b
                 << ") at positions " << i << "," << i + 1 << "\n";
            return false; // hard fail: don't "invent" bits
        }
    }
    return true;
}

int main() {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    int opt = 1;
    if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        ::close(sock);
        return 1;
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_port        = htons(9000);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(sock, (sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        ::close(sock);
        return 1;
    }

    if (::listen(sock, 1) < 0) {
        perror("listen");
        ::close(sock);
        return 1;
    }

    cout << "Receiver: Waiting for sender...\n";

    sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_sock = ::accept(sock, (sockaddr *)&client, &client_len);
    if (client_sock < 0) {
        perror("accept");
        ::close(sock);
        return 1;
    }
    cout << "Receiver: Sender connected.\n";

    // ---- Read ALL data until sender closes ----
    string payload;
    payload.reserve(8192);

    char buf[4096];
    while (true) {
        ssize_t n = ::read(client_sock, buf, sizeof(buf));
        if (n < 0) { perror("read"); ::close(client_sock); ::close(sock); return 1; }
        if (n == 0) break; // EOF (sender closed)
        payload.append(buf, buf + n);
    }

    cout << "Receiver: Received " << payload.size() << " bytes.\n";

    if (payload.empty()) {
        cerr << "Receiver: Empty payload.\n";
        ::close(client_sock);
        ::close(sock);
        return 1;
    }

    // Convert '0'/'1' chars to bits (STRICT validation)
    vector<int> enc_bits;
    enc_bits.reserve(payload.size());

    for (char c : payload) {
        if (c == '0' || c == '1') enc_bits.push_back(c - '0');
        else {
            cerr << "Receiver: Invalid char in payload (not 0/1): '" << c << "'\n";
            cout << "Receiver: ERROR DETECTED!\n";
            ::close(client_sock);
            ::close(sock);
            return 0;
        }
    }

    cout << "Receiver: Encoded bits = " << enc_bits.size() << "\n";

    // Manchester decode
    vector<int> all_bits;
    if (!manchester_decode(enc_bits, all_bits)) {
        cout << "Receiver: ERROR DETECTED (Manchester decode failed)\n";
        ::close(client_sock);
        ::close(sock);
        return 0;
    }

    cout << "Receiver: Decoded bits (data + CRC) = " << all_bits.size() << "\n";

    // Need at least 16 CRC bits + at least 1 data bit
    if (all_bits.size() < 17) {
        cerr << "Receiver: Not enough bits (need >= 17: at least 1 data + 16 CRC).\n";
        cout << "Receiver: ERROR DETECTED!\n";
        ::close(client_sock);
        ::close(sock);
        return 0;
    }

    // Split data and CRC
    size_t data_len = all_bits.size() - 16;
    vector<int> data_bits(all_bits.begin(), all_bits.begin() + (long)data_len);

    uint16_t recv_crc = 0;
    for (size_t i = data_len; i < all_bits.size(); ++i) {
        recv_crc = (uint16_t)((recv_crc << 1) | (all_bits[i] & 1));
    }

    uint16_t calc_crc = crc16(data_bits);

    cout << "Receiver: Data bits length = " << data_len << "\n";
    cout << "Receiver: Received CRC = 0x" << hex << recv_crc
         << ", Calculated CRC = 0x" << calc_crc << dec << "\n";

    if (recv_crc == calc_crc) cout << "Receiver: NO ERROR DETECTED.\n";
    else                      cout << "Receiver: ERROR DETECTED!\n";

    ::close(client_sock);
    ::close(sock);
    return 0;
}
