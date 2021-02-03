<?hh
// TODO: this test only ensures that the file currently passes the typechecker
// over time as we implement typechecker features, it will split into smaller
// tests that error
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
}

// Can technically use this function to readonlyfy things
function ro<T>(T $x) : readonly T {
  return $x;
}

async function returns_readonly() : readonly Awaitable<Foo> {
  return new Foo();
}

async function returns_normal(): Awaitable<Foo> {
  return new Foo();
}

async function expressions(readonly Foo $x) : readonly Awaitable<Foo> {
  // lambda
  $f = (readonly Foo $y) : readonly Foo ==> { return $y;};

  // function
  $z = function(readonly Foo $f) : readonly Foo {
    return $f;
  };

  $r = readonly new Foo();
  $w = $z(readonly $r);
  $readonly_first = readonly await returns_normal();
  $await_readonly = await readonly returns_readonly();

  // This should parse, but won't typecheck and will fail at runtime
  // because returns_readonly returns a readonly value, it must be explicitly
  // cast to readonly
  $readonly_first_readonly = readonly await returns_readonly();

  // like below
  // note that the first readonly is redundant, just testing parsing.
  $correct = readonly await readonly returns_readonly();


  return $x;
}



class Bar {
  // TODO: readonly on properties
  public  /* readonly */  Foo $x;


  // TODO: this readonly applies to the function but not the property yet
  public function __construct(
    public readonly Foo $y,
    ) {
    $this->x = new Foo();
  }
  // TODO: readonly on methods
  public /* readonly */ function getFoo() : void {
    // TODO: readonly annotation on lambda
    $f = /* <<readonly>> */  (Foo $y) ==> {return $y;};
    // TODO: readonly annotation on anonymous function
    $z = /* readonly */ function(Foo $f)  : void {
    };
  }

}
