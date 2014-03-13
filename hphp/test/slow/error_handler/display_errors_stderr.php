<?php

ini_set('display_errors', 'stderr');
var_dump(ini_get('display_errors'));
fclose(STDOUT);
trigger_error('Should see', E_USER_NOTICE);

ini_set('display_errors', 'stdout');
trigger_error('Should not see', E_USER_NOTICE);
