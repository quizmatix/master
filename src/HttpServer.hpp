/*
 * Quizmatix Master Server
 * Copyright (C) 2025 bariscodefx
 * This software is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * For more information, see README.md and LICENSE
 */

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>

class HttpServer
{
private:
    int server_fd;
    int port;

public:
    HttpServer(int p) : port(p), server_fd(-1) {}

    ~HttpServer()
    {
        if (server_fd != -1)
        {
            close(server_fd);
        }
    }

    bool start()
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            std::cerr << "Failed to create socket\n";
            return false;
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            std::cerr << "Bind failed\n";
            return false;
        }

        if (listen(server_fd, 10) < 0)
        {
            std::cerr << "Listen failed\n";
            return false;
        }

        std::cout << "Server listening on port " << port << std::endl;
        return true;
    }

    void run()
    {
        while (true)
        {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

            if (client_fd >= 0)
            {
                std::thread(&HttpServer::handleClient, this, client_fd).detach();
            }
        }
    }

private:
    void handleClient(int client_fd)
    {
        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0)
        {
            close(client_fd);
            return;
        }

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n";

        send(client_fd, response.c_str(), response.length(), 0);
        close(client_fd);
    }
};
