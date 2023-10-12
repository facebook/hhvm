<?hh // strict
class Demo {
  public string $s = '';
  public readonly string $t = '';

  public function setS(string $s): this {
    $this->s = $s;
    return $this;
  }

  public function setT(string $t): this {
    $this->t = $t;
    return $this;
  }
  public function exposeBug(): void {
    $s = readonly "123";
    // Error is ->setS($$) regarding passing readonly when the parameter is mutable
    "234" |> $this->setS($$)->setT($s |> HH\Readonly\as_mut($$));
  }
}


<<__EntryPoint>>
  function foo(): void {
    $x = new Demo();
    $x->exposeBug();
    echo "Done!\n";
  }
