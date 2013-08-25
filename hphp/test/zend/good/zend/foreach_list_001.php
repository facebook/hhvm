<?php

foreach(array(array(1,2), array(3,4)) as list($a, $b)) {
    var_dump($a . $b);
}

$array = array(
    array('a', 'b'),
    array('c', 'd'),
);

foreach ($array as list($a, $b)) {
    var_dump($a . $b);
}


$multi = array(
    array(array(1,2), array(3,4)),
    array(array(5,6), array(7,8)),
);

foreach ($multi as list(list($a, $b), list($c, $d))) {
    var_dump($a . $b . $c . $d);
}

foreach ($multi as $key => list(list($a, $b), list($c, $d))) {
    var_dump($key . $a . $b . $c . $d);
}


?>