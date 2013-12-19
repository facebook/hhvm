<?php
ini_set('magic_quotes_gpc', 1);

echo "*** Test by calling method or function with deprecated option ***\n";
var_dump(get_cfg_var( 'magic_quotes_gpc' ) );

?>