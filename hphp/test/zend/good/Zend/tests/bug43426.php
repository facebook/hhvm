<?hh

function foo2($d) {}

<<__EntryPoint>> function main(): void {
$c = 1; // doesn't matter
call_user_func(fun("foo2"), $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c,
$c,
 $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c, $c);

echo "ok\n";
}
