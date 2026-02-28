<?hh

class X {
  function foo() :mixed{
    if (!$this->bar()) return;
    return $this::bar();
  }

  function bar() :mixed{ return __hhvm_intrinsics\launder_value(1); }
}


<<__EntryPoint>>
function main_cls_ref_dce() :mixed{
var_dump((new X)->foo());
}
