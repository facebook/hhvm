<?hh

class X {
  <<__DynamicallyCallable>> static function foo() { var_dump(__METHOD__); }
}

function test(string $s) {
  if (is_a($s, 'X', true)) {
    call_user_func(varray[$s, 'foo']);
  }
  if (is_a($s, 'X', false)) {
    call_user_func(varray[$s, 'foo']);
  }
  if (is_a($s, 'X')) {
    call_user_func(varray[$s, 'foo']);
  }
}


<<__EntryPoint>>
function main_isa_inference() {
  test('X');
}
