<?php

var_dump(preg_match('/<.*>/', '<aa> <bb> <cc>', $m));
var_dump($m);

var_dump(preg_match('/<.*>/U', '<aa> <bb> <cc>', $m));
var_dump($m);

var_dump(preg_match('/(?U)<.*>/', '<aa> <bb> <cc>', $m));
var_dump($m);

?>