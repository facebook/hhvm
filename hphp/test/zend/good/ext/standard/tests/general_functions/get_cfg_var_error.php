<?php

echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

try { var_dump(get_cfg_var( 'session.use_cookies', 'session.serialize_handler' ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(get_cfg_var(  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


