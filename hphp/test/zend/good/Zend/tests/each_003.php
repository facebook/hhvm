<?php

$a = array(array(), &$a);

var_dump(each(&$a[1]));

