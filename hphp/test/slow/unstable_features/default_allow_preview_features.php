<?hh
// unstable features that are marked preview are allowed even without -vAllowUnstableFeatures
<<file:__EnableUnstableFeatures('readonly')>>

/* Testing readonly syntax below: note that HHVM ignores readonly right
 now so this is just testing that it compiles ignoring it */
class Foo {
  public int $prop = 4;
}

function ro<T>(T $x) : readonly T {
  return $x;
}

async function returns_readonly() : readonly Awaitable<Foo> {
  return readonly new Foo();
}

async function returns_normal(): Awaitable<Foo> {
  return new Foo();
}

async function expressions(readonly Foo $x) : readonly Awaitable<Foo> {
  $f = (readonly Foo $y) : readonly Foo ==> { return $y;};

  $z = function(readonly Foo $f) : readonly Foo {
    return $f;
  };

  $r = readonly new Foo();
  $w = readonly $z(readonly $r);
  $readonly_first = readonly await returns_normal();
  $await_readonly = await readonly returns_readonly();
  $correct = readonly await readonly returns_readonly();


  return $x;
}



class Bar {
  public readonly Foo $x;

  public function __construct(
    public readonly Foo $y,
    ) {
    $this->x = new Foo();
  }
  public readonly function getFoo() : void {
    $f = /* <<readonly>> */  (Foo $y) ==> {return $y;};
    $z = readonly function(Foo $f)  : void {
    };
  }

}
<<__EntryPoint>>
async function test() : Awaitable<void> {
  $x = await readonly returns_readonly();
  echo $x->prop;
}
