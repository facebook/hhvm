<?hh

type S = shape('a' => int, 'b' => bool);
newtype NS = shape('a' => int, 'b' => bool);

class C {
  const type TCShape = shape('a' => int, 'b' => bool);
}

function redundant_unsafe_casts(string $str, shape('a' => int, 'b' => bool) $s): void {
  HH\FIXME\UNSAFE_CAST<arraykey, string>($str);
  HH\FIXME\UNSAFE_CAST<shape(...), S>($s);
  HH\FIXME\UNSAFE_CAST<NS, S>($s);
  HH\FIXME\UNSAFE_CAST<shape(...), NS>($s);
  HH\FIXME\UNSAFE_CAST<C::TCShape, NS>($s);
  HH\FIXME\UNSAFE_CAST<shape(...), C::TCShape>($s);
  HH\FIXME\UNSAFE_CAST<S, C::TCShape>($s);
}
