<?php

var_dump(getenv('herp'));
putenv('herp=derp');
var_dump(parse_ini_string('foo=${herp}'));
