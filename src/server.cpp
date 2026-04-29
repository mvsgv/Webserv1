#include "../includes/inc.hpp"

/*                  s = socket(domain, type, protocol);
AF_INET = Internet domain IPv4
SOCK_STREAM = TCP
              Provides sequenced, reliable, two-way, connection-based
              byte streams.  An out-of-band data transmission mechanism
              may be supported.
0 = default protocol  */

int start_server(){

    int server_fd;
    int client_socket;
    struct sockaddr_in address;             // Added for configuration
    socklen_t addrlen = sizeof(address);    // Added for accept()
    // Supposing BUFFER_SIZE is defined in your hpp, otherwise statically allocate it:
    // char buffer[BUFFER_SIZE] = {0};

    //CREATE SOCKET
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0 ){
        std::cerr <<  "Error: socket failed" << std::endl;
        return 1;
    }

    //ADDRESS CONFIGURATION
    std::memset(&address, 0,sizeof(address));
    address.sin_family = AF_INET;// -> je travaille en IPv4
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 -> j'accepte les connexions partout ->j'ecoute partout
    address.sin_port = htons(PORT); // Host TO Network Short -> suis sur le port 8080(mon serveur)

    //BIND -> attach socket to the address
    // Added comma between server_fd and (struct sockaddr *)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        std::cerr << "Error: bind failed" << std::endl;
        close(server_fd); // Clean up on error
        return 1;
    }

    //LISTEN
    if (listen(server_fd, 10) < 0)//-> waiting for client to connect, with max 10 pending connexioons
    {
        std::cerr << "Error: listen failed" << std::endl;
        close(server_fd); // Clean up on error
        return 1;
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    while (true){
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0){
            std::cerr << "Error: accept failed" << std::endl;
            continue;//ignore and wait for next client
        }
        
        char buffer[BUFFER_SIZE];
        std::memset(buffer, 0, sizeof(buffer)); // Clear previous buffer
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read > 0){
            buffer[bytes_read] = '\0'; // Null-terminate for string printing
            std::cout << "\n--REQUEST SENT--\n"<< buffer <<std::endl;
        }
        
        // Very important: free the client socket!
        close(client_socket);
    }
    
    close(server_fd);
    return 0;
}
