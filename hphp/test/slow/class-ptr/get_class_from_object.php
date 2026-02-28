<?hh

class C {
  public static function f(): void { echo "called\n"; }
}

<<__EntryPoint>>
function main(): void {
  $c = new C();

  //// CASE: subtypeOf(BObj)
  $cc = HH\get_class_from_object($c);

  // lines 12 and 16 folded away to
  // FCallClsMethodD <SkipRepack SkipCoeffectsCheck> 0 1 "" "" - "" "C" "f"
  $cc::f();

  //// CASE: couldBe(BObj)
  $m = __hhvm_intrinsics\launder_value($c);

  // FCallFuncD <SkipRepack SkipCoeffectsCheck> 1 1 "" "" - "" "HH\\get_class_from_object"
  // AssertRATStk 0 Cls
  $cm = HH\get_class_from_object($m);

  // FCallClsMethodM <SkipRepack SkipCoeffectsCheck> 0 1 "" "" - "" "" DontLogAsDynamicCall "f"
  $cm::f();

  //// CASE: !couldBe(BObj)
  $f = 3.14;

  // Double 3.14
  // FCallFuncD <SkipRepack SkipCoeffectsCheck> 1 1 "" "" - "" "HH\\get_class_from_object"
  // StaticAnalysisError
  $cdead = HH\get_class_from_object($f);

  // No bytecode emitted for these lines
  $cdead::f();
  echo "unreachable\n";
}
