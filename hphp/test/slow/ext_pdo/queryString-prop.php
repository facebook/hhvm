<?php

$rc = new ReflectionClass('PDOStatement');
$rp = $rc->getProperty('queryString');
echo $rp;
