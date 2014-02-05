<?hh

class Queryable {

  public function executeQuery() {
    $args = func_get_args();
    $args[1] = "closure";
    return $args;
  }
}

function print_result($e) {
  if (is_string($e)) {
    echo 'string(', strlen($e), ')', ' with \000 printed as <0>:', "\n",
      strtr($e, [chr(0) => '<0>']), "\n";
  } else {
    var_dump($e);
  }
}
