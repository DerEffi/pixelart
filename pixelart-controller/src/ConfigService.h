#include <Config.h>

#ifndef CONFIG_SERVICE_H_
#define CONFIG_SERVICE_H_

class ConfigService {
    private:
        Config _settings;
    public:
        Config getSettings() const { return _settings; }
        void Init();
};

#endif