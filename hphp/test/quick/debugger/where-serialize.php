<?hh


class Thing implements Serializable {
  public $foo;

  public function __construct($x) {
    $this->foo = $x;
  }

  public function serialize() {
    bar($this);
    return serialize($this->foo);
  }

  public function unserialize($str) {
    $this->foo = unserialize($str);
  }
}

function bar($b) {
  var_dump($b);
}

bar(new Thing(1));
