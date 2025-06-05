#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

class Config
{
private:
    std::unordered_map<std::string, std::string> configData;
    std::string configFile;

public:
    Config(const std::string &filename) : configFile(filename)
    {
        loadConfig();
    }

    void loadConfig()
    {
        std::ifstream file(configFile);
        std::string line;

        while (std::getline(file, line))
        {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            size_t pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                configData[key] = value;
            }
        }
    }

    std::string getString(const std::string &key, const std::string &defaultValue = "")
    {
        auto it = configData.find(key);
        return (it != configData.end()) ? it->second : defaultValue;
    }

    int getInt(const std::string &key, int defaultValue = 0)
    {
        auto it = configData.find(key);
        if (it != configData.end())
        {
            return std::stoi(it->second);
        }
        return defaultValue;
    }

    bool getBool(const std::string &key, bool defaultValue = false)
    {
        auto it = configData.find(key);
        if (it != configData.end())
        {
            std::string value = it->second;
            return (value == "true" || value == "1" || value == "yes");
        }
        return defaultValue;
    }

    void setValue(const std::string &key, const std::string &value)
    {
        configData[key] = value;
    }

    void saveConfig()
    {
        std::ofstream file(configFile);
        for (const auto &pair : configData)
        {
            file << pair.first << " = " << pair.second << std::endl;
        }
    }
};