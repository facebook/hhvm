<?hh

/* test #1: numeric data */
function add_up($running_total, $current_value)
:mixed{
    echo "running_total is ".(int)$running_total.", current_value is {$current_value}\n";
    $running_total = HH\Lib\Legacy_FIXME\cast_for_arithmetic($running_total);
    $running_total += HH\Lib\Legacy_FIXME\cast_for_arithmetic($current_value) * HH\Lib\Legacy_FIXME\cast_for_arithmetic($current_value);
    return $running_total;
}
function foo ($a, $b)
:mixed{
    return (string)($a) . (string)($b);
}
function rsum($v, $w)
:mixed{
    $v = HH\Lib\Legacy_FIXME\cast_for_arithmetic($v);
    $v += HH\Lib\Legacy_FIXME\cast_for_arithmetic($w);
    return $v;
}
function rmul($v, $w)
:mixed{
    $v *= $w;
    return $v;
}
<<__EntryPoint>> function main(): void {
$numbers = varray [2,3,5,7];
$total = array_reduce($numbers, add_up<>);
print "Total is $total\n";

/* test #2: string data */
$a = vec["a", "b", "c"];
var_dump(array_reduce($a, foo<>));

/* test #3: basic test (used to leak memory) */
$a = vec[1, 2, 3, 4, 5];
$x = vec[];
$b = array_reduce($a, rsum<>);
$c = array_reduce($a, rmul<>, 10);
$d = array_reduce($x, rsum<>, 1);

var_dump($b, $c, $d);
}
