<?hh
class A {};
class B extends A {};
class C extends B {};
<<__EntryPoint>>
function main() {
  if (isset($GLOBALS['foo'])) {
    include "pseudo-cycles-1.inc";
    include "pseudo-cycles-2.inc";
    include "pseudo-cycles-3.inc";
  }
  var_dump(new C());
}
