<?hh

class C {
  public $x;
  public function __toDebugDisplay() { return "lol"; }
}

function test($thing) {
  echo "==== " . get_class($thing) . " ====\n";
  // 9 == VariableSerializer::Type::DebuggerSerialize
  var_dump(__hhvm_intrinsics\serialize_with_format($thing, 9));
}

<<__EntryPoint>>
function main() {
  set_error_handler(($errno, $string) ==> {
    if ($errno === E_NOTICE &&
        strpos($string, "Lval on missing array element") !== false) {
      throw new Exception("lol");
    }
    return false;
  });

  test(new C());
}
