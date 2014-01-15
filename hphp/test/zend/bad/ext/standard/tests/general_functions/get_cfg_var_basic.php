<?php


echo "*** Test by calling method or function with its expected arguments ***\n";
var_dump(get_cfg_var( 'session.use_cookies' ) );
var_dump(get_cfg_var( 'session.serialize_handler' ) );
var_dump(get_cfg_var( 'session.save_handler' ) );

?>