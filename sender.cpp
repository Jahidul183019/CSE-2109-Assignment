#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// ---------- CRC-16 over bit vector (poly = 0x1021) ----------
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

// ---------- Manchester encode: 0 -> 10, 1 -> 01 ----------
vector<int> manchester_encode(const vector<int> &bits) {
    vector<int> encoded;
    encoded.reserve(bits.size() * 2);
    for (int b : bits) {
        if (b == 0) {
            encoded.push_back(1);
            encoded.push_back(0);
        } else {
            encoded.push_back(0);
            encoded.push_back(1);
        }
    }
    return encoded;
}

// ---------- Error injection helpers (on frame_bits!) ----------
void flip_bit(vector<int> &bits, size_t pos) {
    if (pos < bits.size()) bits[pos] ^= 1;
}

void flip_burst(vector<int> &bits, size_t pos, size_t len) {
    if (pos >= bits.size()) return;
    if (pos + len > bits.size()) len = bits.size() - pos;
    for (size_t i = 0; i < len; ++i) {
        bits[pos + i] ^= 1;
    }
}

int main() {
    // 1. Data bits
    string data_str = "1011001100110101";  // 16 data bits
    vector<int> data_bits;
    for (char c : data_str) {
        if (c == '0' || c == '1') data_bits.push_back(c - '0');
    }

    cout << "Sender: Data bits = " << data_str << "\n";

    // 2. Compute CRC and build frame = data + CRC
    uint16_t crc = crc16(data_bits);
    cout << "Sender: CRC-16 = 0x" << hex << crc << dec << "\n";

    vector<int> frame_bits = data_bits;
    for (int i = 15; i >= 0; --i) {
        frame_bits.push_back((crc >> i) & 1);
    }
    cout << "Sender: Frame bits (data + CRC) = "
         << frame_bits.size() << "\n";

    // 3. Choose error on FRAME bits
    cout << "\nError test cases:\n"
         << "0 = No error\n"
         << "1 = Single bit error\n"
         << "2 = Two isolated single-bit errors\n"
         << "3 = Odd number of errors (3 bits)\n"
         << "4 = Burst error length 8\n"
         << "5 = Burst error length 17\n"
         << "6 = Burst error length 22\n"
         << "Enter choice: ";

    int choice = 0;
    cin >> choice;

    switch (choice) {
        case 0:
            cout << "Sender: No error.\n";
            break;
        case 1:
            cout << "Sender: Single bit error at frame bit 10.\n";
            flip_bit(frame_bits, 10);
            break;
        case 2:
            cout << "Sender: Two isolated errors at frame bits 5 and 20.\n";
            flip_bit(frame_bits, 5);
            flip_bit(frame_bits, 20);
            break;
        case 3:
            cout << "Sender: Odd number of errors at frame bits 2, 10, 25.\n";
            flip_bit(frame_bits, 2);
            flip_bit(frame_bits, 10);
            flip_bit(frame_bits, 25);
            break;
        case 4:
            cout << "Sender: Burst error of length 8 starting at frame bit 8.\n";
            flip_burst(frame_bits, 8, 8);
            break;
        case 5:
            cout << "Sender: Burst error of length 17 starting at frame bit 8.\n";
            flip_burst(frame_bits, 8, 17);
            break;
        case 6:
            cout << "Sender: Burst error of length 22 starting at frame bit 5.\n";
            flip_burst(frame_bits, 5, 22);
            break;
        default:
            cout << "Sender: Invalid choice, sending with no error.\n";
            break;
    }

    // 4. Manchester encode the (possibly corrupted) frame bits
    vector<int> encoded_bits = manchester_encode(frame_bits);
    cout << "Sender: Manchester encoded bits = "
         << encoded_bits.size() << "\n";

    // 5. Convert to '0'/'1' characters
    string payload;
    payload.reserve(encoded_bits.size());
    for (int b : encoded_bits) payload.push_back(b ? '1' : '0');

    // 6. Send via TCP socket
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port   = htons(9000);

    if (::inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
        perror("inet_pton");
        ::close(sock);
        return 1;
    }

    if (::connect(sock, (sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        ::close(sock);
        return 1;
    }

    cout << "Sender: Connected, sending data...\n";

    size_t total = 0;
    while (total < payload.size()) {
        ssize_t n = ::send(sock,
                           payload.data() + total,
                           payload.size() - total,
                           0);
        if (n <= 0) { perror("send"); break; }
        total += (size_t)n;
    }

    cout << "Sender: Sent " << total << " bytes.\n";

    ::close(sock);
    return 0;
}
