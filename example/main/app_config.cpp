#include "app_config.h"

const config_state<app_config> *const app_config::STATE =
    &(*new config_state_set<app_config>())
         .add_field(&app_config::num, "/num", nullptr, config_state_disable_persistence)
         .add_field(&app_config::pin, "/pin")
         .add_field(&app_config::str, "/str")
         .add_value_list(&app_config::num_list, "/num_list")
         .add_value_list(&app_config::str_list, "/str_list");
