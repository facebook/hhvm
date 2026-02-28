<?hh

type s = shape(?'x' => mixed, ...);
type tt = shape(?'y' => mixed, ...);

class C<T> {
  public function __construct(T $_) {}
}

function test(s $s): C<tt> {
  return new C($s);
}
