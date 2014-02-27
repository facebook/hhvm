<?hh

class Queryable {

  public function executeQuery() {
    $args = func_get_args();
    $serializedAST = $args[0];
    //echo $serializedAST."\n";
    $ast = unserialize($serializedAST);
    //var_dump($ast);
    $cmToPHP = new HH\CodeModel\CodeModelToPHP();
    $args[0] = $ast->accept($cmToPHP);
    $args[1] = "closure";
    return $args;
  }
}

function print_result($e) {
  if (is_string($e)) {
    echo $e."\n";
  } else {
    var_dump($e);
  }
}
