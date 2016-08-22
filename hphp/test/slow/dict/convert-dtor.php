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
  var_dump(dict(dict[$i => new Dtor($i),
                     $i+1 => new Dtor($i+1),
                     $i+2 => new Dtor($i+2)]));
  echo "====================================================\n";

  $i += 3;
  var_dump(vec(dict[$i => new Dtor($i),
                    $i+1 => new Dtor($i+1),
                    $i+2 => new Dtor($i+2)]));
  echo "====================================================\n";

  $i += 3;
  try {
    var_dump(keyset(dict[$i => new Dtor($i),
                         $i+1 => new Dtor($i+1),
                         $i+2 => new Dtor($i+2)]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  echo "====================================================\n";

  $i += 3;
  var_dump((array)dict[$i => new Dtor($i),
                       $i+1 => new Dtor($i+1),
                       $i+2 => new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((bool)dict[$i => new Dtor($i),
                      $i+1 => new Dtor($i+1),
                      $i+2 => new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((int)dict[$i => new Dtor($i),
                     $i+1 => new Dtor($i+1),
                     $i+2 => new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((float)dict[$i => new Dtor($i),
                       $i+1 => new Dtor($i+1),
                       $i+2 => new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((string)dict[$i => new Dtor($i),
                        $i+1 => new Dtor($i+1),
                        $i+2 => new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump((object)dict[$i => new Dtor($i),
                        $i+1 => new Dtor($i+1),
                        $i+2 => new Dtor($i+2)]);
  echo "====================================================\n";

  $i += 3;
  var_dump(new Vector(dict[$i => new Dtor($i),
                           $i+1 => new Dtor($i+1),
                           $i+2 => new Dtor($i+2)]));
  echo "====================================================\n";

  $i += 3;
  var_dump(new Map(dict[$i => new Dtor($i),
                        $i+1 => new Dtor($i+1),
                        $i+2 => new Dtor($i+2)]));
  echo "====================================================\n";
}

main();
