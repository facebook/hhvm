<?php

$x = new stdclass;
var_dump($x->$y =& $z);
