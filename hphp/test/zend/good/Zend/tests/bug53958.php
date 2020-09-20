<?hh
<<__EntryPoint>> function main(): void {
$a = 1;
$fn1 = function() use ($a) {echo "$a\n"; $a++;};
$fn2 = function() use ($a) {echo "$a\n"; $a++;};
$a = 5;
$fn1(); // 1
$fn2(); // 1
$fn1(); // 1
$fn2(); // 1
}
