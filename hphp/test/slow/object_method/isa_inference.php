<?hh

class X {
  <<__DynamicallyCallable>> static function foo() :mixed{ var_dump(__METHOD__); }
}

function test(string $s) :mixed{
  if (is_a($s, 'X', true)) {
    call_user_func(vec[$s, 'foo']);
  }
  if (is_a($s, 'X', false)) {
    call_user_func(vec[$s, 'foo']);
  }
  if (is_a($s, 'X')) {
    call_user_func(vec[$s, 'foo']);
  }
}


<<__EntryPoint>>
function main_isa_inference() :mixed{
  test('X');
}
