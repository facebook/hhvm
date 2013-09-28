<?php
echo "--SQLite3::open--\n";
$ro = new ReflectionClass('SQLite3');
$mo = $ro->getMethod('open');
$params = $mo->getParameters();
var_dump($params[1]->getDefaultValue());
var_dump(str_replace(' ', '', $params[1]->getDefaultValueText()));

