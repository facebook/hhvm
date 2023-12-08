<?hh
<<file: __EnableUnstableFeatures('function_references')>>

class C {
  public static function static_meth(arraykey $x): arraykey {
    return $x;
  }
  public function meth(arraykey $x): arraykey {
    return $x;
  }
  public static function static_generic_meth<T>(T $x): T {
    return $x;
  }
  public function generic_meth<T>(T $x): T {
    return $x;
  }
}

function top(arraykey $x): arraykey {
  return $x;
}
function generic_top<T>(T $x): T {
  return $x;
}

function acceptFun((function(string): mixed) $f): void {
}
function acceptMeth((function(C, string): mixed) $f): void {
}
function acceptFunRef(
  HH\FunctionRef<(readonly function(arraykey): arraykey)> $f,
): void {
}
function acceptMethRef(
  HH\FunctionRef<(function(C, arraykey): arraykey)> $f,
): void {
}
function acceptFunRef2(
  HH\FunctionRef<(readonly function(string): mixed)> $f,
): void {}

function applyFunRef(
  HH\FunctionRef<(readonly function(arraykey): arraykey)> $f,
): void {
  ($f)(3);
  // Should be able to pass function value as a subtype of the expected type
  acceptFun($f);
  // Or as an exact reference
  acceptFunRef($f);
  // But this should be an error
  acceptFunRef2($f);
}
function expectAK(arraykey $_): void {}

<<__EntryPoint>>
function testit(): void {
  $tf = top<>;
  expectAK($tf(3));
  acceptFun($tf);
  acceptFunRef($tf);
  // Should reject
  acceptFunRef2($tf);

  $sm = C::static_meth<>;
  expectAK($sm(3));
  acceptFun($sm);
  acceptFunRef($sm);
  // Should reject
  acceptFunRef2($sm);

  $m = meth_caller(C::class, 'meth');
  expectAK($m(new C(), 3));
  acceptMeth($m);
  acceptMethRef($m);

  $lam = (arraykey $x): arraykey ==> $x;
  acceptFun($lam);
  // Should be rejected
  acceptFunRef($lam);

  $gt = generic_top<>;
  acceptFun($gt);
  // Should be rejected, because generic
  acceptFunRef($gt);

  $gsm = C::static_generic_meth<>;
  acceptFun($gsm);
  // Should be rejected, because generic
  acceptFunRef($gsm);

  $gm = meth_caller(C::class, 'generic_meth');
  acceptMeth($gm);
  // Should be rejected, because generic
  acceptMethRef($gm);
}
