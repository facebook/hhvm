<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
}

// Can technically use this function to readonlyfy things
function ro<T>(T $x) : readonly T {
  return $x;
}

function basic(readonly Foo $x) : readonly Foo {
  // lambda
  $f = (readonly Foo $y) : readonly Foo ==> { return $y;};

  // function
  $z = function(readonly Foo $f) : readonly Foo {
    return $f;
  };

  // TODO: readonly expressions (should have same precedence as
  // await as an expression)
  // $r = readonly new Foo();

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
