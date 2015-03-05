<?php
$rc = new ReflectionFunction('ereg');
var_dump($rc->isDeprecated());
