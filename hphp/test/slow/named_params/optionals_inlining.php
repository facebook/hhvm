<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>


<<__ALWAYS_INLINE>>
function foo(named int $a, named int $b = 4, named string $c = "c") {
    var_dump(vec[$a, $b, $c]);
}

function bar(named int $x = 42, named string $y = "y") {
    foo(a=3);
    foo(a=3, b=$x);
    foo(a=3, c=$y);
    foo(a=3, b=$x, c=$y);
}

<<__EntryPoint>>
function main() {
  bar();
  bar(x=21);
  bar(y="yyy");
  bar(x=21, y="yyy");
}
