<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(__hhvm_intrinsics\is_lazy_class(HH\classname_from_string_unsafe("C")));
  var_dump(__hhvm_intrinsics\is_lazy_class(HH\classname_from_string_unsafe("WHOAMI")));
}
