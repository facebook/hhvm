<?hh

function err($x) { throw new Exception(); }
function foo() {
  $x = varray[darray['asd' => true]];
  try {
    $x[0]['asd'][] = 2;
  } catch (Exception $e) {
    var_dump(is_array($x));
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_minstr_throw_004() {
set_error_handler(fun('err'));
foo();
}
