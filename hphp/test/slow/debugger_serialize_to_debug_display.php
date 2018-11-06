<?hh

class C {
  public function __toDebugDisplay() { return "lol"; }
}

class D {
  public function __toDebugDisplay() { throw new Exception("lol"); }
}

class E {
  public function __toDebugDisplay() { throw new Error(2.0); }
}

class F {
  public function __toDebugDisplay() { __hhvm_intrinsics\trigger_oom(true); }
}

function test($thing) {
  echo "==== " . get_class($thing) . " ====\n";
  // 9 == VariableSerializer::Type::DebuggerSerialize
  var_dump(__hhvm_intrinsics\serialize_with_format($thing, 9));
}

<<__EntryPoint>>
function main() {
  test(new C());
  test(new D());
  test(new E());
  test(new F());
}
