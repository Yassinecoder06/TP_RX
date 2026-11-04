# 🌐 Socket Programming in C (`TP_RX`)

Client-server network programming implementations in **C** using POSIX sockets (`sys/socket.h`, `netinet/in.h`, `arpa/inet.h`) across **TCP**, **UDP**, and **HTTP**.

---

## 📂 Included Programs

| File | Protocol | Description |
| :--- | :--- | :--- |
| `tcp_time_server.c` | TCP | Iterative TCP server returning the current system date and time to connected clients. |
| `tcp_time_server_fork.c` | TCP | Concurrent multi-client TCP time server using `fork()` to handle simultaneous connections. |
| `tcp_time_client.c` | TCP | TCP client that connects to the time server and displays the server's response. |
| `udp_time_server.c` | UDP | Connectionless UDP datagram server responding to time requests via `recvfrom()` / `sendto()`. |
| `udp_time_client.c` | UDP | UDP client sending datagram requests to the UDP time server. |
| `http_client.c` | HTTP / TCP | Raw socket HTTP client capable of sending HTTP requests and reading server headers/body. |

---

## 🛠️ Compilation & Usage

Compile any source file using `gcc`:

```bash
gcc tcp_time_server.c -o tcp_time_server
gcc tcp_time_server_fork.c -o tcp_time_server_fork
gcc tcp_time_client.c -o tcp_time_client
gcc udp_time_server.c -o udp_time_server
gcc udp_time_client.c -o udp_time_client
gcc http_client.c -o http_client
```

### Example: Running the TCP Server & Client

```bash
# Terminal 1 — Start the server
./tcp_time_server 8080

# Terminal 2 — Connect with the client
./tcp_time_client 127.0.0.1 8080
```
