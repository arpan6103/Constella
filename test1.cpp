#include <iostream>
#include <iomanip>
#include <arpa/inet.h>  // for htonl, ntohl (Linux/macOS)
#include <cstring>

// Print the individual bytes of a 32‑bit integer (big‑endian order in output)
void print_bytes(const char* label, uint32_t value) {
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
    std::cout << label << " = 0x" << std::hex << std::setw(8) << std::setfill('0') << value
              << "  (bytes: ";
    for (int i = 0; i < 4; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
        if (i < 3) std::cout << ' ';
    }
    std::cout << ")\n" << std::dec;
}

int main() {
    // Original message length (in host byte order)
    uint32_t original_len = 0x01020304;    // 16909060 decimal
    
    std::cout << "=== Host Byte Order (original) ===\n";
    print_bytes("original_len", original_len);
    
    // Convert to network byte order (big‑endian)
    uint32_t network_len = htonl(original_len);
    std::cout << "\n=== After htonl() (network byte order) ===\n";
    print_bytes("network_len", network_len);
    
    // Convert back to host byte order
    uint32_t back_to_host = ntohl(network_len);
    std::cout << "\n=== After ntohl() (back to host) ===\n";
    print_bytes("back_to_host", back_to_host);
    
    // Check that the round‑trip works
    std::cout << "\nRound‑trip successful: " 
              << (original_len == back_to_host ? "YES" : "NO") << std::endl;
    
    // Demonstration of why this matters: send a message length over a socket
    std::cout << "\n--- Practical example: sending 1024 bytes ---\n";
    uint32_t msg_len = 1024;  // 1024 in decimal = 0x00000400
    print_bytes("msg_len (host)", msg_len);
    
    uint32_t len_for_network = htonl(msg_len);
    print_bytes("len_for_network (network order)", len_for_network);
    
    std::cout << "\nThe receiver will apply ntohl() to get back 1024.\n";
    
    return 0;
}