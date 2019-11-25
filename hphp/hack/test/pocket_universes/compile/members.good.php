<?hh // strict

class C {
  enum E {
    case int val;
    :@A0(val = 42);
    :@A1(val = 43);
    :@A2(val = 44);
    :@A3(val = 45);
  }
}

function foo(vec<C:@E> $v) : void {
  foreach ($v as $x) {
    echo C:@E::val($x);
    echo "\n";
  }
}

<<__EntryPoint>>
function main() : void {
  $v = C:@E::Members();
  foo($v);
}
