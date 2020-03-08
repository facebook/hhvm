<?hh // strict

namespace NS_num;

class C {
  const num COUNT = 100;
  private num $prop = 0;

  public function setProp(num $val): void {
    $this->prop = $val;
  }

  public function getProp(): num {
    return $this->prop;
  }

  public function __construct() {
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    $this->prop = 1.2;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    $this->prop = 6;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    $this->prop += 0.0;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    $this->prop = 16;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    $this->prop *= 1.0;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    $this->prop = PHP_INT_MAX;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");

    ++$this->prop;
    echo "num " . $this->prop . (is_int($this->prop) ? " is int\n" : " is float\n");
  }
}

function main (): void {
  $c = new C();
}

/* HH_FIXME[1002] call to main in strict*/
main();
