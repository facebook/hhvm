<?hh


class c {
  private $prop = 'ohai';

  public function doit() {
    unset($this->prop);
  }

  public function showProp() {
    var_dump($this->prop);
  }

  public function setProp() {
    $this->prop = 'yo';
  }

  public function __get($name) {
    return 'prop-'.$name;
  }

  public function __set($name, $value) {
    echo "setting $name to $value\n";
  }
}

<<__EntryPoint>> function main(): void {
  $c = new c;
  $c->showProp();
  $c->setProp();

  $c->doit();
  $c->showProp();
  $c->setProp();
}
