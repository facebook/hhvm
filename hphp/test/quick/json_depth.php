<?hh
$a = array(array('A'), array('B'), array('C'), array('D'));
$b = array($a, $a, $a, $a);
$c = array($b, $b, $b, $b);
$d = array($c, $c, $c, $c);
$e = array($d, $d, $d, $d);
var_dump(json_encode($e, 0, 3));
var_dump(json_encode($e, 0, 4));
var_dump(json_encode($e, 0, 5));
var_dump(json_encode($e, 0, 6));
var_dump(json_encode($e, 0, 7));
var_dump(json_encode($e));
$E = json_decode(json_encode($e));
var_dump($E == $e);
