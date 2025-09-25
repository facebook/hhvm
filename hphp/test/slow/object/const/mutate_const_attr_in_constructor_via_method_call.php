<?hh

class C {
  <<__Const>>
  public int $b = 0;

  public function __construct(<<__Const>> public int $a) {
    $this->b = 0;
    $this->incrB();
  }

  public function incrB(): void {
    $this->b++;
  }

  public function incrA(): void {
    $this->a++;
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new C(4);
  try {
    $c->incrB();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  try {
    $c->incrA();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
