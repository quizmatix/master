/*
 * Quizmatix Master Server
 * Copyright (C) 2025 bariscodefx
 * This software is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * For more information, see README.md and LICENSE
 */

#include "HttpServer.hpp"

int main()
{
    Config *config = new Config("Config.txt");
    HttpServer server(config->getInt("HTTP_PORT"), config);

    if (!server.start())
    {
        return 1;
    }

    server.run();

    return 0;
}