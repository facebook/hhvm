<?hh // strict
class Demo {
  public function getReadonly(string $s): readonly string {
    return $s;
  }
  public function empty(string $s): void {}
  public function exposeBug(): void {
    $s = "123";
    $this->empty($s);
    // Error: This expression is readonly. It is incompatible with this parameter, which is mutable
    $s = readonly $this->getReadonly($s);
  }
  public function exposeBug2(): void {
    $v = vec["123", "234"];
    foreach ($v as $s) {
      // Error here as well
      $this->empty($s);
      // Error here
      $s = readonly $this->getReadonly($s);
    }
  }
}
<<__EntryPoint>>
  function foo(): void {
    $y = new Demo();
    $y->exposeBug();
    $y->exposeBug2();
    echo "Done!\n";
  }
