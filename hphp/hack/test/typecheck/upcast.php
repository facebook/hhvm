<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(int $x) : void {
  $y = $x upcast num;
  hh_show($y);
}

function g<T as int>(T $x) : void {
  $y = $x upcast num;
  $z = $x upcast int;
  hh_show($y);
  hh_show($z);
}

function g2<T super num>(num $x) : void {
  $y = $x upcast T;
  hh_show($y);
}

function h(vec<int> $x) : void {
  $y = $x upcast vec<num>;
  hh_show($y);
}

function expr(int $x) : void {
  4 + $x as num + 1;
}
