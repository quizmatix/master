/*
 * Quizmatix Master Server
 * Copyright (C) 2025 bariscodefx
 * This software is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * For more information, see README.md and LICENSE
 */

#ifndef WEBSOCKETPP_ECHO_SERVER_HANDLER_HPP
#define WEBSOCKETPP_ECHO_SERVER_HANDLER_HPP

class echo_handler : public server::handler
{
    void on_message(connection_ptr con, std::string msg)
    {
        con->write(msg);
    }
};

#endif // WEBSOCKETPP_ECHO_SERVER_HANDLER_HPP