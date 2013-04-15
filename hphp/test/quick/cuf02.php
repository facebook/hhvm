<?php
class C {  public function foo() { echo "C::foo\n"; var_dump(get_called_class()); }
  public static function bar() { echo "C::bar\n"; var_dump(get_called_class()); }
}
class D extends C {
  public function foo() { echo "D::foo\n"; var_dump(get_called_class()); }
  public static function bar() { echo "D::bar\n"; var_dump(get_called_class()); }
}
class E {
  public function foo() { echo "E::foo\n"; var_dump(get_called_class()); }
  public static function bar() { echo "E::bar\n"; var_dump(get_called_class());
  }
}


function main() {
  call_user_func('C::foo');
  call_user_func('D::foo');
  call_user_func('E::foo');

  call_user_func('C::bar');
  call_user_func('D::bar');
  call_user_func('E::bar');
}
main();

