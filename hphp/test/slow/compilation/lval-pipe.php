<?hh

function bar($cc) {
  return Map { 'a' => $cc };
}

class X {
  public static function foo(string $cc) {
    $x = bar($cc);
    if ($x === null) {
      return;
    }

    return $x |> array_shift(&$$);
  }
}

function main($t) {
  return X::foo($t);
}


<<__EntryPoint>>
function main_lval_pipe() {
var_dump(main("hello"));
}
