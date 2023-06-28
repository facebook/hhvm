<?hh

class C { public function __toString()[] :mixed{ return "lol"; } }

<<__EntryPoint>>
function test() :mixed{
  var_dump(__hhvm_intrinsics\id_string(new C()));
}
