<?hh // strict

class X {
  function foo() {
    if (!$this->bar()) return;
    return $this::bar();
  }

  function bar() { return __hhvm_intrinsics\launder_value(1); }
}


<<__EntryPoint>>
function main_cls_ref_dce() {
var_dump((new X)->foo());
}
