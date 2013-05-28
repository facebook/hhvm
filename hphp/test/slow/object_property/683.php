<?php

$one = array('cluster'=> 1, 'version'=>2);
var_dump(isset($one->cluster));
var_dump(empty($one->cluster));
$two = 'hello';
var_dump(isset($two->scalar));
