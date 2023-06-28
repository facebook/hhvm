<?hh
function f(inout $a) :mixed{ $a++; print $a; }
function g($a) :mixed{ $a++; print $a; }
<<__EntryPoint>> function main(): void {
$a = f<>;
if (0) $a(3);
$val4 = 4;
if (0) f(inout $val4);
g(5);
print "\n";
$val3 = 3;
f(inout $val3);
}
