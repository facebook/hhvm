<?

class FibberFactoryFacade {
  public function f($val) {
    if ($val <= 1) {
      return 1;
    }
    return $this->f($val - 2) + $this->f($val - 1);
  }
}

$fibber = new FibberFactoryFacade();
for ($i = 0; $i < 35; $i++) {
  echo $fibber->f($i) . "\n";
}
