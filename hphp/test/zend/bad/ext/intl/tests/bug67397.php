<?php

function ut_main()
{
    $ret = var_export(ut_loc_get_display_name(str_repeat('*', 256), 'en_us'), true);
    $ret .= "\n";
    $ret .= var_export(intl_get_error_message(), true);
    return $ret;
}

include_once( 'ut_common.inc' );
ut_run();
?>
