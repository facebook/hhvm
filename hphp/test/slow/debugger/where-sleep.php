<?hh


class Sleeper {
  public $foo;

  public function __construct($x) {
    $this->foo = $x;
  }

  public function __sleep() {
    echo "sleep\n";
    bar($this);
    return varray['foo'];
  }

  public function __wakeup() {
    echo "wakeup\n";
  }
}

function bar($b) {
  var_dump($b);
}
<<__EntryPoint>>
function entrypoint_wheresleep(): void {

  bar(new Sleeper(1));
}
