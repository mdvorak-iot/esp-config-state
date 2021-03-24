#include "app_config.h"

const config_state<app_config> *const app_config::STATE =
    &(*new config_state_set<app_config>())
         .add_field("/num", "num", &app_config::num, config_state_disable_persistence)
         .add_field("/pin", "pin", &app_config::pin)
         .add_field("/str", "str", &app_config::str)
         .add_value_list("/num_list", "num_list", &app_config::num_list)
         .add_value_list("/str_list", "str_list", &app_config::str_list);
