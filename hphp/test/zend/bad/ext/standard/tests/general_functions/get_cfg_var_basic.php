<?php
ini_set('session.save_handler', files);

ini_set('session.serialize_handler', php);

ini_set('session.use_cookies', 0);



echo "*** Test by calling method or function with its expected arguments ***\n";
var_dump(get_cfg_var( 'session.use_cookies' ) );
var_dump(get_cfg_var( 'session.serialize_handler' ) );
var_dump(get_cfg_var( 'session.save_handler' ) );

?>