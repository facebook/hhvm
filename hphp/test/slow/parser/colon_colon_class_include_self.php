<?hh

class C {
  static function foo() { include 'colon_colon_class_include_self.inc'; }
}

<<__EntryPoint>>
function main_colon_colon_class_include_self() {
C::foo();
}
