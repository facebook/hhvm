<?hh

class Foo {

  // Pure constructor with default values desugars into a function
  // with the expression `$this->prop_int = prop_int` in the body.
  // We are treating this as *not* subject to the pure function check
  // on object property writes.
  public function __construct(
    public int $prop_int = 0,
   )[] {}

}

function pure_function(Foo $x)[] : bool {
  // Local variable assignment that should not error
  $message = 'hello';

  // Exprs that look like but are not assignments and should not error
  if ($x->prop_int == 4) {}
  if ($x->prop_int === 4) {}
  if ($x->prop_int <= 4) {}
  if ($x->prop_int >= 4) {}
  if ($x->prop_int != 4) {}

  $x->prop_int == 4;
  $x->prop_int === 4;
  $x->prop_int <= 4;
  $x->prop_int >= 4;
  $x->prop_int != 4;

  return $x->prop_int == 4;
}


<<__EntryPoint>>
function main(): void {

  pure_function(new Foo(9));

  echo "Done\n";

}
