<?hh

class C { public function __toString() { return "lol"; } }

<<__EntryPoint>>
function test() {
  var_dump(__hhvm_intrinsics\id_string(new C()));
}
