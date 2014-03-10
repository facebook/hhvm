<?php

ini_set('display_errors', '2');
var_dump(ini_get('display_errors'));
fclose(STDOUT);
trigger_error('Should see', E_USER_NOTICE);

// STDOUT, which we just closed - shouldn't be rendered
ini_set('display_errors', '1');
trigger_error('Should not see', E_USER_NOTICE);
