<?hh

class C { static function foo() {} }
class D extends C {
  static function bar() { return class_meth('parent', 'foo'); }
}

<<__EntryPoint>>
function main() {
  D::bar();
  echo "FAIL\n";
}
