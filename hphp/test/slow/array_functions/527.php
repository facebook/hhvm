<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

var_dump(array_unique(array(array(1,2), array(1,2), array(3,4),)));
