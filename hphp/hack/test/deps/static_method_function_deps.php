<?hh
interface I {
  public function interface_only_method(): void;
}
class C implements I {
  public static function static_counter(): void { obj_only(); }
  public function interface_only_method(): void { bar(); }
  public function class_only_method(): void {
    $C_obj = new C();
    foo($C_obj);
  }
}

class A {
  public function call_static_method(): void {
    C::static_counter();
  }
}

function foo(I $I_obj): void {
  C::static_counter();
  $I_obj->interface_only_method();
}
function bar(): void {
  $C_obj = new C();
  $C_obj->class_only_method();
  foo($C_obj);
}
function obj_only(): void{
  $C_obj = new C();
}
