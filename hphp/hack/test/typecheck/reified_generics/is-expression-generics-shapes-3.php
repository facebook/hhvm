<?hh

class C<reify Ta> {}

function f(): void {
  $c = new C<shape('a' => int, 'b' => string, ...)>();

  // just wildcard
  $c is C<_>;

  // shape with wildcard
  $c is C<shape('a' => int, 'b' => string)>;
  $c is C<shape('b' => string, 'a' => int)>;
  $c is C<shape('a' => _, 'b' => string)>;
  $c is C<shape('a' => int, 'b' => _)>;
  $c is C<shape('a' => string, 'b' => _)>;

  // missing
  $c is C<shape('a' => int)>;

  // extra
  $c is C<shape('a' => int, 'b' => string, 'c' => string)>;

  // optional
  $c is C<shape('a' => int, 'b' => string, ?'c' => string)>;
  $c is C<shape(?'a' => int, 'b' => string)>;

  // unknown fields
  $c is C<shape('a' => int, 'b' => string, ...)>;
}
