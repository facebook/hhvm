<?php

var_dump(ob_start('bad_func'));
var_dump('this prints');
var_dump(ob_end_clean());
