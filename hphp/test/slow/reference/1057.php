<?php

$a = array(1, 'a');
 $b = $a;
 foreach ($b as $k => &$v) {
 $v = 'ok';
}
 var_dump($a, $b);
