#include "app_config.h"

const config_state<app_config> *const app_config::STATE =
    &(*new config_state_set<app_config>())
         .add_field("/num", &app_config::num)
         .add_field("/pin", &app_config::pin)
         .add_field("/str", &app_config::str)
         .add_value_list("/num_list", &app_config::num_list)
         .add_value_list("/str_list", &app_config::str_list);
