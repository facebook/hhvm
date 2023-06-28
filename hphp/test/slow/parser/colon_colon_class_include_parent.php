<?hh

class C {
  static function foo() :mixed{ include 'colon_colon_class_include_parent.inc'; }
}

<<__EntryPoint>>
function main_colon_colon_class_include_parent() :mixed{
C::foo();
}
