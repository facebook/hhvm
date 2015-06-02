<?php

$array = array( 1, true, "string" );
xdebug_var_dump( $array ); echo "\n\n";
ini_set('xdebug.var_display_max_depth', 0);
xdebug_var_dump( $array ); echo "\n\n";
ini_set('xdebug.var_display_max_depth', -1);
ini_set('xdebug.var_display_max_data', 0);
xdebug_var_dump( $array ); echo "\n\n";
ini_set('xdebug.var_display_max_children', 0);
ini_set('xdebug.var_display_max_data', -1);
xdebug_var_dump( $array ); echo "\n\n";
