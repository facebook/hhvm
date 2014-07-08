<?php

function ini_get_wrapper( $setting ) {
    return ini_get( $setting );
}

var_dump(ini_get_wrapper('some_non_existant_setting'));
var_dump(ini_get_wrapper('some_non_existant_setting'));
