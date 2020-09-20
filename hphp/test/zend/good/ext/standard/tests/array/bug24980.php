<?hh

/* test #1: numeric data */
function add_up($running_total, $current_value)
{
    echo "running_total is ".(int)$running_total.", current_value is {$current_value}\n";
    $running_total += $current_value * $current_value;
    return $running_total;
}
function foo ($a, $b)
{
    return $a . $b;
}
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
<<__EntryPoint>> function main(): void {
$numbers = varray [2,3,5,7];
$total = array_reduce($numbers, fun('add_up'));
print "Total is $total\n";

/* test #2: string data */
$a = varray["a", "b", "c"];
var_dump(array_reduce($a, fun("foo")));

/* test #3: basic test (used to leak memory) */
$a = varray[1, 2, 3, 4, 5];
$x = varray[];
$b = array_reduce($a, fun("rsum"));
$c = array_reduce($a, fun("rmul"), 10);
$d = array_reduce($x, fun("rsum"), 1);

var_dump($b, $c, $d);
}
