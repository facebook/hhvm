<?hh

// lambdas in class bodies

class bar {
  private $x = "asd";

  public function __destruct() {
    echo "~bar()\n";
  }

  public function foo() {
    return array_map(
      $y ==> $this->x,
      array(1,2,3,4)
    );
  }

  public function getClos() {
    // Doesn't capture $this
    return $x ==> $x;
  }

  public function grabThis() {
    return () ==> $this->x;
  }
}

function main() {
  var_dump((new bar)->foo());
  $k = (new bar)->getClos();
  // dtor prints here
  echo $k("sup\n");

  echo "holding this:\n";
  $k = (new bar)->grabThis();
  echo $k() . "\n";
  unset($k);
  echo "Done\n";
}

main();
