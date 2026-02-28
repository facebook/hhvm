<?hh

function err($x) :mixed{ throw new Exception(); }
function foo() :mixed{
  $x = vec[dict['asd' => true]];
  try {
    $x[0]['asd'][] = 2;
  } catch (Exception $e) {
    var_dump(is_array($x));
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_minstr_throw_004() :mixed{
set_error_handler(err<>);
foo();
}
