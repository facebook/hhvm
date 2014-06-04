<?php

var_dump(server_get_custom_bool_setting("NonExistentConfigSetting", true));
var_dump(server_get_custom_bool_setting("NonExistentConfigSetting", false));
