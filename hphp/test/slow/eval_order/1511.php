<?php

$a = array(array($id = 1, $id), array($id = 2, $id));
var_dump($a);
$a = array(+($id = 1), $id, -($id = 2), $id,            !($id = 3), $id, ~($id = 4), $id,            isset($a[$id = 5]), $id);
var_dump($a);
