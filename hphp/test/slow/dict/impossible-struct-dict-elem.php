<?hh

function foo(shape(...) $x) {
  // $x is profiled as StructDict<"foo":Obj">
  // When it gets inlined with a static dict input, we check if it is
  // the profiled StructDict and if so, use StructDictElemAddr+LdMem
  // to obtain the Obj, which becomes Bottom, since the input is static
  // and can't contain Obj.
  return $x['foo'];
}

<<__EntryPoint>>
function main() {
  // profile the arg as StructDict<"foo":Obj>
  foo(shape('foo' => new stdClass()));

  // $s is a static dict
  $s = rand() % 2 ? shape('foo' => 42) : shape('foo' => 47);
  if (HH\jit_jumpstarted()) {
    // after retranslate-all, send it to foo()
    foo($s);
  }
  echo "done";
}
