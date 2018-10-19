<?hh

class C { public function __toString() { return "lol"; } }

<<__EntryPoint>>
function test() {
  var_dump(fun(new C()));
}
