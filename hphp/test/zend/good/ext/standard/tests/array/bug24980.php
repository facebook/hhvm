<?php
/* test #1: numeric data */
function add_up($running_total, $current_value)
{
	echo "running_total is ".(int)$running_total.", current_value is {$current_value}\n";
	$running_total += $current_value * $current_value;
	return $running_total;
}
                        
$numbers = array (2,3,5,7);
$total = array_reduce($numbers, 'add_up');
print "Total is $total\n";

/* test #2: string data */
$a = array("a", "b", "c");
function foo ($a, $b)
{
	return $a . $b;
}
var_dump(array_reduce($a, "foo"));
                        
/* test #3: basic test (used to leak memory) */
function rsum($v, $w)
{
	$v += $w;
	return $v;
}
function rmul($v, $w)
{
	$v *= $w;
	return $v;
}
$a = array(1, 2, 3, 4, 5);
$x = array();
$b = array_reduce($a, "rsum");
$c = array_reduce($a, "rmul", 10);
$d = array_reduce($x, "rsum", 1);

var_dump($b, $c, $d);
?>