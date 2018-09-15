<?hh // strict

type s = shape(?'x' => mixed, ...);
type t = shape(?'y' => mixed, ...);

class C<T> {
  public function __construct(T $_) {}
}

function test(s $s): C<t> {
  return new C($s);
}
