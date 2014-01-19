<?php

$a = array(1,2, '' => 'foo');
unset($a[null]);
var_dump($a);
