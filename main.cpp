#include "webserver.hpp"

// Wojciech Sniady, 322993

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "wrong number of arguments\n");
		return EXIT_FAILURE;
    }
    int port = std::stoi(argv[1]);
    std::string path = argv[2];
    if (!port_validation(port)) {
        fprintf(stderr, "wrong port number given\n");
		return EXIT_FAILURE;
    }
    if (!directory_validation(path)) {
        fprintf(stderr, "wrong directory path given\n");
		return EXIT_FAILURE;
    }
    std::cout<<"Server listening on port "<<port<<", source directory: "<<path<<"\n";
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "bind error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
    if (listen(sockfd, MAX_LISTENERS_NO) < 0)
	{
		fprintf(stderr, "listen error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
    while (true)
    {
        struct sockaddr_in client_address;
        unsigned int client_address_len = sizeof(client_address);
        int client_socket;
        if ((client_socket = accept(sockfd, (struct sockaddr *) &client_address, (unsigned int *) &client_address_len)) < 0)
        {
            fprintf(stderr, "accept error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
        while (true)
        {
            fd_set descriptors;
            FD_ZERO(&descriptors);
            FD_SET(client_socket, &descriptors);
            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            int ready = select(client_socket + 1, &descriptors, nullptr, nullptr, &tv);
            if (ready < 0)
            {
                fprintf(stderr, "select error: %s\n", strerror(errno));
                break;
            }
            else if (ready == 0) {
                break;
            }
            char message_received[MESSAGE_LENGTH];
            ssize_t packet_len = recv(client_socket, message_received, MESSAGE_LENGTH, 0);
            if (packet_len < 0)
            {
                fprintf(stderr, "recv error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
            else if (packet_len == 0) {
                break;
            }
            std::string request(message_received);
            bool conn_close = process_request(client_socket, message_received, path);
            if (conn_close) {
                break;
            }
        }
        if (close(client_socket) < 0) {
            fprintf(stderr, "close error: %s\n", strerror(errno));
        }
    }
}