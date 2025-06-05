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
#include <vector>
#include <json/json.h>
#include <random>
#include <algorithm>
#include <mutex>
#include "classes/Server.hpp"
#include "Config.hpp"

class HttpServer
{
private:
    int server_fd;
    int port;
    std::vector<Server> serverPool;
    Config *config;
    std::mutex serverPoolMutex; // Thread safety için

public:
    HttpServer(int p, Config *cfg) : port(p), server_fd(-1), config(cfg) {}

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

        // Cleanup thread başlat
        std::thread(&HttpServer::cleanupOfflineServers, this).detach();

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
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0)
        {
            close(client_fd);
            return;
        }

        std::string request(buffer);
        std::string response;

        if (request.find("POST /add") == 0)
        {
            response = handleAddServer(request);
        }
        else if (request.find("POST /heartbeat") == 0)
        {
            response = handleHeartbeat(request);
        }
        else if (request.find("GET /") == 0)
        {
            response = handleGetServers();
        }
        else
        {
            response = createHttpResponse(404, "Not Found", "Endpoint not found");
        }

        send(client_fd, response.c_str(), response.length(), 0);
        close(client_fd);
    }

    std::string handleAddServer(const std::string &request)
    {
        // Extract JSON body from POST request
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart == std::string::npos)
        {
            return createHttpResponse(400, "Bad Request", "Invalid request format");
        }

        std::string body = request.substr(bodyStart + 4);

        try
        {
            Json::Value root;
            Json::Reader reader;

            if (!reader.parse(body, root))
            {
                return createHttpResponse(400, "Bad Request", "Invalid JSON");
            }

            // Check registry key
            std::string providedKey = root.get("registry_key", "").asString();
            std::string expectedKey = config->getString("SERVER_REGISTRY_KEY", "");

            if (providedKey != expectedKey || expectedKey.empty())
            {
                return createHttpResponse(401, "Unauthorized", "Invalid registry key");
            }

            // Extract server address
            std::string address = root.get("address", "").asString();
            if (address.empty())
            {
                return createHttpResponse(400, "Bad Request", "Address is required");
            }

            std::lock_guard<std::mutex> lock(serverPoolMutex);

            // Check if server already exists
            for (auto &server : serverPool)
            {
                if (server.getAddress() == address)
                {
                    // Sunucu zaten varsa, yeni token ver ve last seen güncelle
                    std::string newToken = generateToken();
                    server.setToken(newToken);
                    server.updateLastSeen();

                    Json::Value responseJson;
                    responseJson["message"] = "Server re-registered successfully";
                    responseJson["token"] = newToken;

                    Json::StreamWriterBuilder builder;
                    std::string jsonString = Json::writeString(builder, responseJson);

                    std::cout << "Server re-registered: " << address << " with token: " << newToken << std::endl;
                    return createHttpResponse(200, "OK", jsonString, "application/json");
                }
            }

            // Generate token for new server
            std::string token = generateToken();

            // Add server to pool
            Server newServer(address);
            newServer.setToken(token);
            serverPool.push_back(newServer);

            Json::Value responseJson;
            responseJson["message"] = "Server registered successfully";
            responseJson["token"] = token;

            Json::StreamWriterBuilder builder;
            std::string jsonString = Json::writeString(builder, responseJson);

            std::cout << "Server added to pool: " << address << " with token: " << token << std::endl;
            return createHttpResponse(200, "OK", jsonString, "application/json");
        }
        catch (const std::exception &e)
        {
            return createHttpResponse(500, "Internal Server Error", "Error processing request");
        }
    }

    std::string handleHeartbeat(const std::string &request)
    {
        // Extract JSON body from POST request
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart == std::string::npos)
        {
            return createHttpResponse(400, "Bad Request", "Invalid request format");
        }

        std::string body = request.substr(bodyStart + 4);

        try
        {
            Json::Value root;
            Json::Reader reader;

            if (!reader.parse(body, root))
            {
                return createHttpResponse(400, "Bad Request", "Invalid JSON");
            }

            std::string token = root.get("token", "").asString();
            if (token.empty())
            {
                return createHttpResponse(400, "Bad Request", "Token is required");
            }

            std::lock_guard<std::mutex> lock(serverPoolMutex);

            // Find server by token and update last seen
            for (auto &server : serverPool)
            {
                if (server.getToken() == token)
                {
                    server.updateLastSeen();
                    return createHttpResponse(200, "OK", "Heartbeat received");
                }
            }

            return createHttpResponse(401, "Unauthorized", "Invalid token");
        }
        catch (const std::exception &e)
        {
            return createHttpResponse(500, "Internal Server Error", "Error processing heartbeat");
        }
    }

    std::string handleGetServers()
    {
        try
        {
            std::lock_guard<std::mutex> lock(serverPoolMutex);

            Json::Value root(Json::arrayValue);

            for (const auto &server : serverPool)
            {
                if (server.isOnline(30)) // 30 saniye timeout (3 heartbeat)
                {
                    Json::Value serverObj;
                    serverObj["address"] = server.getAddress();
                    root.append(serverObj);
                }
            }

            Json::StreamWriterBuilder builder;
            std::string jsonString = Json::writeString(builder, root);

            return createHttpResponse(200, "OK", jsonString, "application/json");
        }
        catch (const std::exception &e)
        {
            return createHttpResponse(500, "Internal Server Error", "Error retrieving servers");
        }
    }

    std::string createHttpResponse(int statusCode, const std::string &statusText,
                                   const std::string &body, const std::string &contentType = "text/plain")
    {
        std::ostringstream response;
        response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "\r\n";
        response << body;

        return response.str();
    }

    std::string generateToken()
    {
        const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.size() - 1);

        std::string token;
        for (int i = 0; i < 32; ++i)
        {
            token += chars[dis(gen)];
        }

        return token;
    }

    void cleanupOfflineServers()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(60)); // Her dakika kontrol et

            std::lock_guard<std::mutex> lock(serverPoolMutex);

            auto it = serverPool.begin();
            while (it != serverPool.end())
            {
                if (!it->isOnline(30)) // 30 saniye timeout
                {
                    std::cout << "Removing offline server: " << it->getAddress() << std::endl;
                    it = serverPool.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    // Get server pool (for debugging/monitoring)
    const std::vector<Server> &getServerPool() const
    {
        return serverPool;
    }
};
