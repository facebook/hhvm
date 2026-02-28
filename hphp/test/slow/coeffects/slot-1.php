<?hh

class A {
  abstract const ctx C = [write_props];
  <<__NEVER_INLINE>>
  function f()[this::C] :mixed{ echo "in f\n"; }
}

class B1 extends A {
  const ctx C = [zoned];
}

class B2 extends A {
  const ctx C = [defaults];
}

<<__EntryPoint>>
function main() :mixed{
  (__hhvm_intrinsics\launder_value(new B1))->f();
  (__hhvm_intrinsics\launder_value(new B2))->f();
}
