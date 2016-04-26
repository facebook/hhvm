<?hh
class C {
  private static float $foo = $bar + ; // Parse error
  public function M($bar) // $bar should be found regardless of the error.
  {
    $abc = $bar; // $bar should be found regardless of the error.
  }
  public function N()
  {
    $abc = $bar; // Should not be found
  }
}
