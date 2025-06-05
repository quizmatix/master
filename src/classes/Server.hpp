/*
 * Quizmatix Master Server
 * Copyright (C) 2025 bariscodefx
 * This software is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * For more information, see README.md and LICENSE
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <chrono>

class Server
{
private:
    std::string address;
    std::string token;
    std::chrono::steady_clock::time_point lastSeen;

public:
    Server();
    Server(const std::string &address);

    // Getters
    const std::string &getAddress() const;
    const std::string &getToken() const;
    std::chrono::steady_clock::time_point getLastSeen() const;

    // Setters
    void setAddress(const std::string &address);
    void setToken(const std::string &token);
    void updateLastSeen();

    // Utility
    bool isOnline(int timeoutSeconds = 300) const; // 5 dakika default timeout
};

#endif // SERVER_HPP