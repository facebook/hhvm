<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->get());
var_dump($c->getActualMaximum());
var_dump($c->getActualMinimum());

var_dump($c->get(-1));
var_dump($c->getActualMaximum(-1));
var_dump($c->getActualMinimum(-1));

var_dump($c->get("s"));
var_dump($c->getActualMaximum("s"));
var_dump($c->getActualMinimum("s"));

var_dump($c->get(1, 2));
var_dump($c->getActualMaximum(1, 2));
var_dump($c->getActualMinimum(1, 2));