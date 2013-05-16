<?php
set_time_limit(5);

$f=12.3;
var_dump($f);
var_dump(serialize($f));
var_dump(unserialize(serialize($f)));

$f=-12.3;
var_dump($f);
var_dump(serialize($f));
var_dump(unserialize(serialize($f)));

$f=-INF;
var_dump($f);
var_dump(serialize($f));
var_dump(unserialize(serialize($f)));

$f=INF;
var_dump($f);
var_dump(serialize($f));
var_dump(unserialize(serialize($f)));

$f=NAN;
var_dump($f);
var_dump(serialize($f));
var_dump(unserialize(serialize($f)));

?>