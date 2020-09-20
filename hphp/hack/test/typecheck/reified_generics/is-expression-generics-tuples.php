<?hh

class C<reify Ta> {}

function f(): void {
  $c = new C<(int, string)>();

  // just wildcard
  $c is C<_>;

  // tuple with wildcard
  $c is C<(int, string)>;
  $c is C<(int, _)>;
  $c is C<(_, string)>;
  $c is C<(_, _)>;
  $c is C<(_, int)>;
}
