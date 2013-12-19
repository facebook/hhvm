<?php
ini_set('session.save_handler', files);

ini_set('session.serialize_handler', php);

ini_set('session.use_cookies', 0);


echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

var_dump(get_cfg_var( 'session.use_cookies', 'session.serialize_handler' ) );
var_dump(get_cfg_var(  ) );


?>