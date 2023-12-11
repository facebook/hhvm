<?hh

// lambdas in class bodies

class bar {
  private $x = "asd";

  public function foo() :mixed{
    return array_map(
      $y ==> $this->x,
      vec[1,2,3,4]
    );
  }

  public function getClos() :mixed{
    // Doesn't capture $this
    return $x ==> $x;
  }

  public function grabThis() :mixed{
    return () ==> $this->x;
  }
}

<<__EntryPoint>> function main(): void {
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
