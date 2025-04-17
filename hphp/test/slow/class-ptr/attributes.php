<?hh

class TakesStr implements HH\ClassAttribute {
  public function __construct(string $input)[]: void {
    __hhvm_intrinsics\debug_var_dump_lazy_class($input);
  }
}

class TakesClsname implements HH\ClassAttribute {
  public function __construct(classname<mixed> $input)[]: void {
    __hhvm_intrinsics\debug_var_dump_lazy_class($input);
  }
}

class TakesCls implements HH\ClassAttribute {
  public function __construct(class<mixed> $input)[]: void {
    __hhvm_intrinsics\debug_var_dump_lazy_class($input);
  }
}

<<TakesStr(nameof C_str)>>
class C_str {}
<<TakesStr(C_cls::class)>>
class C_cls {}

<<TakesClsname(D_cls::class)>>
class D_cls {}
<<TakesClsname(nameof D_str)>>
class D_str {}

<<TakesCls(E_cls::class)>>
class E_cls {}
<<TakesCls(nameof E_str)>>
class E_str {}

<<__EntryPoint>>
function main(): void {
  $c_str = new ReflectionClass(nameof C_str);
  var_dump($c_str->getAttributeClass(TakesStr::class));
  $c_cls = new ReflectionClass(nameof C_cls);
  var_dump($c_cls->getAttributeClass(TakesStr::class));

  $d_cls = new ReflectionClass(nameof D_cls);
  var_dump($d_cls->getAttributeClass(TakesClsname::class));
  $d_str = new ReflectionClass(nameof D_str);
  var_dump($d_str->getAttributeClass(TakesClsname::class));

  $e_cls = new ReflectionClass(nameof E_cls);
  var_dump($e_cls->getAttributeClass(TakesCls::class));
  $e_str = new ReflectionClass(nameof E_str);
  var_dump($e_str->getAttributeClass(TakesCls::class));
}
