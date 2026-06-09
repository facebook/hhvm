<?hh
// Regression test for a JIT SIGABRT when Eval.NoticeOnCreateDynamicProp is set
// to true and raises a notice in the lazy deserialize path with SyncOptions::None.
//
// NOTE: triggering the lazy path requires the classes to be lazy-eligible
// (persistent RDS) and the read to be JITed.
// Run this test in repo mode (-r).

class Inner {
  public int $a = 1;
}

class Outer {
  public ?Inner $inner = null;
}

// Isolated so the JIT compiles the property read - this is the
// DeserializeLazyProp site that lacked a fixup.
function readInner(Outer $o): mixed {
  return $o->inner;
}

<<__EntryPoint>>
function main(): void {
  if (apc_exists('lazy_dynprop_outer')) {
    // Each fetch yields a fresh object whose `inner` property is still lazy;
    // reading it from JITed code runs DeserializeLazyProp. Loop enough to JIT.
    for ($i = 0; $i < 5000; $i++) {
      $o = __hhvm_intrinsics\apc_fetch_no_check('lazy_dynprop_outer');
      readInner($o);
    }
    echo "DONE\n";
  } else {
    $inner = new Inner();
    $inner->dyn = 42; // dynamic (undeclared) property on the nested object
    $o = new Outer();
    $o->inner = $inner;
    apc_store('lazy_dynprop_outer', $o);
  }
}
