<?php
error_reporting(0);
class C {
  public $a = 2;
}
class D {
  public $b = 1;
}
$c1 = new C;
$c2 = new C;
$c2->a = 3;
$d = new D;
$f = fopen('php://stdout', 'w');
$arr = array('null' => null, 'false' => false, 'true' => true, 'int 0' => 0,
            'int 99' => 99, 'int -1' => -1, 'float 0' => 0.0,'double 99' => (double) 99,
             'float 99' => 99.0, 'float -1' => -1.0, 'INF' => INF,
             '-INF' => -INF, '""' => "", '"0"' => "0",
             '"99"' => "99", '"-1"' => "-1", '"0.0"' => "0.0",
             '"99.0"' => "99.0", '"-1.0"' => "-1.0", 'array()' => array(),
             'array(99)' => array(99), 'object c1' => $c1, 'object c2' => $c2,
             'object d' => $d, 'resource' => $f);
echo "  same lt  lte eq  ne  nw2 gte gt\n\n";
foreach ($arr as $k1 => $v1) {
    foreach ($arr as $k2 => $v2) {
    echo "$k1 cmp $k2:\n  ";
    echo (($v1 === $v2) ? "T    " : "F    ");
    echo (($v1 < $v2) ? "T   " : "F   ");
    echo (($v1 <= $v2) ? "T   " : "F   ");
    echo (($v1 == $v2) ? "T   " : "F   ");
    echo (($v1 >= $v2) ? "T   " : "F   ");
    echo (($v1 > $v2) ? "T   " : "F   ");
    echo "\n";
  }
}
fclose($f);
?>
