<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I { }

<<__SupportDynamicType>>
class C implements I { }

<<__SupportDynamicType>>
function toplevel1(int $x):I {
  // Should only appear once
  hh_show($x);
  return new C();
}

<<__SupportDynamicType>>
function toplevel2(vec<C> $x):I {
  // Should appear twice
  hh_show($x);
  return $x[0];
}

<<__SupportDynamicType>>
async function toplevel3(int $x):Awaitable<I> {
  // Should only appear once
  hh_show($x);
  return new C();
}

<<__SupportDynamicType>>
async function toplevel4(vec<C> $x):Awaitable<I> {
  // Should appear twice
  hh_show($x);
  return $x[0];
}

<<__SupportDynamicType>>
class SD {
  public function meth1(int $x):I {
    // Should only appear once
    hh_show($x);
    return new C();
  }
  public function meth2(vec<C> $x):I {
    // Should appear twice
    hh_show($x);
    return $x[0];
  }
  public async function meth3(int $x):Awaitable<I> {
    // Should only appear once
    hh_show($x);
    return new C();
  }
  public async function meth4(vec<C> $x):Awaitable<I> {
    // Should appear twice
    hh_show($x);
    return $x[0];
  }
}

<<__SupportDynamicType>>
function lambdatest(int $z):void {
  // Should only appear once
  $f1 = (int $x) ==> { hh_show($x); return new C(); };
  // Should appear twice
  $f2 = (vec<C> $x) ==> { hh_show($x); return $x[0]; };
  // Should only appear once
  $f3 = async (int $x) ==> { hh_show($x); return new C(); };
  // Should appear twice
  $f4 = async (vec<C> $x) ==> { hh_show($x); return $x[0]; };
  // Should only appear once
  $f5 = (int $x):I ==> { hh_show($x); return new C(); };
  // Should only appear once
  $f6 = async (int $x):Awaitable<I> ==> { hh_show($x); return new C(); };
}
<<__SupportDynamicType, __ConsistentConstruct>>
abstract class CC<TE as supportdyn<mixed> > {
  final public static function delegateNew(
  ): ~supportdyn<(function(): ~CC<TE>)> {
    $class_name = static::class;
    return () ==> (new $class_name());
  }
}
