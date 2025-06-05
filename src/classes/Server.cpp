/*
 * Quizmatix Master Server
 * Copyright (C) 2025 bariscodefx
 * This software is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * For more information, see README.md and LICENSE
 */

#include "Server.hpp"

Server::Server() : address(""), token(""), lastSeen(std::chrono::steady_clock::now()) {}

Server::Server(const std::string &address) : address(address), token(""), lastSeen(std::chrono::steady_clock::now()) {}

const std::string &Server::getAddress() const
{
    return address;
}

const std::string &Server::getToken() const
{
    return token;
}

std::chrono::steady_clock::time_point Server::getLastSeen() const
{
    return lastSeen;
}

void Server::setAddress(const std::string &address)
{
    this->address = address;
}

void Server::setToken(const std::string &token)
{
    this->token = token;
}

void Server::updateLastSeen()
{
    lastSeen = std::chrono::steady_clock::now();
}

bool Server::isOnline(int timeoutSeconds) const
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastSeen);
    return duration.count() < timeoutSeconds;
}