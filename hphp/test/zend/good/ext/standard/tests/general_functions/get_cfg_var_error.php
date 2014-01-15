<?php

echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

var_dump(get_cfg_var( 'session.use_cookies', 'session.serialize_handler' ) );
var_dump(get_cfg_var(  ) );


?>