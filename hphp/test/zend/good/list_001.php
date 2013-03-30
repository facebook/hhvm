<?php

list($a, list($b)) = array(new stdclass, array(new stdclass));
var_dump($a, $b);

?>