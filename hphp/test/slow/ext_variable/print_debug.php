<?hh
class A {
  public function __toDebugDisplay() :mixed{
    return "Debugging Class A";
  }
}

class B {
  public function __toDebugDisplay() :mixed{
    return "Debugging Class B";
  }
}

<<__EntryPoint>>
function main() {
  $obj_a = new A();
  $obj_b = new B();
  print_debug_display($obj_a);
  print_debug_display($obj_b);
  print_debug_display(vec[$obj_a, $obj_b]);
  print_debug_display(vec[dict["a" => $obj_a, "b" => $obj_b]]);
  print_debug_display(vec[vec[dict["a" => $obj_a, "b" => $obj_b]]]);
}
