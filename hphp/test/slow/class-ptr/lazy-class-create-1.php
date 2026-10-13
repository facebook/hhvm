<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(__hhvm_intrinsics\is_lazy_class(C::class));
  var_dump(__hhvm_intrinsics\is_lazy_class(WHOAMI::class));
}
