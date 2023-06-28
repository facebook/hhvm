<?hh

class Bar {
  public function baz(): this { return $this; }
}

function foo($bar) :mixed{
  // When forming a region, clone will propagate the type of $bar as observed
  // at runtime. If we don't stop the inference, this type will then be used to
  // infer the result of each baz() call, forming a region that contains all
  // three calls.
  //
  // Then we actually try to emit the code, knowing that the cloned $bar is
  // TObj. The first call of baz() would then use a dynamic dispatch, producing
  // a TInitNull. Since the region contains also the second call, we would try
  // to emit it without knowing $this, falling back to the interpreter, which
  // terminates the translation. But the region contains also the third call,
  // so we would emit a bunch of unreachable code full of TBottoms, crashing
  // on random assertions.
  $bar = clone $bar;
  $bar->baz()->baz()->baz();
}

<<__EntryPoint>>
function main() :mixed{
  foo(new Bar());
  echo "ok\n";
}
