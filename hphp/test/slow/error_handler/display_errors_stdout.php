<?php

ini_set('display_errors', 'stdout');
var_dump(ini_get('display_errors'));
fclose(STDERR);
trigger_error('Should see', E_USER_NOTICE);

ini_set('display_errors', 'stderr');
trigger_error('Should not see', E_USER_NOTICE);
