<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(num $x) : void {
  $x upcast int;
}

function g<T as num>(T $x) : void {
  $x upcast int;
}

function h(vec<num> $x) : void {
  $x upcast vec<int>;
}

function i(Vector<int> $x) : void {
  $x upcast Vector<num>;
}
