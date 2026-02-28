<?hh

class C {
  public function __toDebugDisplay() :mixed{ return "lol"; }
}

class D {
  public function __toDebugDisplay() :mixed{ throw new Exception("lol"); }
}

class E {
  public function __toDebugDisplay() :mixed{ throw new Error("err"); }
}

class F {
  public function __toDebugDisplay() :mixed{ __hhvm_intrinsics\trigger_oom(true); }
}

function test($thing) :mixed{
  echo "==== " . get_class($thing) . " ====\n";
  // 9 == VariableSerializer::Type::DebuggerSerialize
  var_dump(__hhvm_intrinsics\serialize_with_format($thing, 9));
}

<<__EntryPoint>>
function main() :mixed{
  test(new C());
  test(new D());
  test(new E());
  test(new F());
}
