<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Dtor {
  public $id;
  function __construct($id) {
    $this->id = $id;
  }
  function __destruct() {
    echo "Dtor::__destruct(" . $this->id . ")\n";
  }
}

function main() {
  $i = 1;
  var_dump(vec(vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]));
  echo "====================================================\n";

  $i += 3;
  var_dump(dict(vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]));
  echo "====================================================\n";

  $i += 3;
  try {
    var_dump(keyset(vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  echo "====================================================\n";

  $i += 3;
  var_dump((array)vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((bool)vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((int)vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((float)vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((string)vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((object)vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump(new Vector(vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]));
  echo "====================================================\n";

  $i += 3;
  var_dump(new Map(vec[new Dtor($i), new Dtor($i+1), new Dtor($i+2)]));
  echo "====================================================\n";
}

main();
