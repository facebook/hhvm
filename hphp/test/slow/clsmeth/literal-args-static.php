<?hh

class C { static function foo() { return class_meth('static', 'foo'); } }
class D extends C {}

<<__EntryPoint>>
function main() {
  D::foo();
  echo "FAIL\n";
}
