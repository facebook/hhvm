<?hh // strict

type A = shape(
  ?'a' => int,
  ...
);

function f<T>(Container<T> $_): void {}

function test(): Vector<A> {
  $v = Vector{shape('a' => 0)};
  f($v);
  return $v;
}
