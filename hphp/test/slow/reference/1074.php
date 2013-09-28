<?php

$a = array(1, array(2,3));
 foreach ($a[1] as &$b) {
 $b++;
}
 var_dump($a);
