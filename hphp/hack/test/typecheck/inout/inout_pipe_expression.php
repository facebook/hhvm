<?hh

function foobar(inout dynamic $x): void {
  // This is OK: `$$` is not appearing in an lvalue position
  1 |> foobar(inout $x[$$]);
  // This is not OK: `$$` is appearing in an lvalue position. This is ...
  // actually really difficult to do in practice, but we disallow it.
  $x |> foobar(inout $$[1]);
}
