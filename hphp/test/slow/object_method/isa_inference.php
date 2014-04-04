<?hh

class X {
  static function foo() { var_dump(__METHOD__); }
}

function test(string $s) {
  if (is_a($s, 'X', true)) {
    call_user_func(array($s, 'foo'));
  }
  if (is_a($s, 'X', false)) {
    call_user_func(array($s, 'foo'));
  }
  if (is_a($s, 'X')) {
    call_user_func(array($s, 'foo'));
  }
}

test('X');
