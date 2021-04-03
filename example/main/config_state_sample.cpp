#include "config_state.h"

struct sample_config
{
    int my_val;

    static std::unique_ptr<config_state<sample_config>> state()
    {
        auto ptr = &(*new config_state_set<sample_config>())
                        .add_field(&sample_config::my_val, "/myVal");
        return std::unique_ptr<config_state<sample_config>>(ptr);
    }

    static const std::unique_ptr<const config_state<sample_config>> STATE;
};

const std::unique_ptr<const config_state<sample_config>> sample_config::STATE = sample_config::state();

extern "C" void app_main()
{
}
