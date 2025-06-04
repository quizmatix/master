/*
 * Quizmatix Master Server
 * Copyright (C) 2025 bariscodefx
 * This software is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * For more information, see README.md and LICENSE
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server
{
private:
    std::string address;
    std::string name;

public:
    Server();
    Server(const std::string &address, const std::string &name);

    // Getters
    const std::string &getAddress() const;
    const std::string &getName() const;

    // Setters
    void setAddress(const std::string &address);
    void setName(const std::string &name);
};

#endif // SERVER_HPP