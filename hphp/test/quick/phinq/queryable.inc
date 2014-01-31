<?hh

class Queryable {

  public function executeQuery() {
    return func_get_args();
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
