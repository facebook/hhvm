<?php

$val = -9.2233720368547758e18;
var_dump($val == unserialize(serialize($val)));
