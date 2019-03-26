<?hh

class asshat {
  private $prop;

  public function __get($k) {
    $this->prop = 'foo';
    return array('foo' => 'gotcha', 1 => 'whoops');
  }

  private function with(&$x, $cond) {
    $x = 1;
    if ($cond) {
      var_dump($this->blah[$x]);
    }
  }
  public function run($cond) {
    $this->with(&$this->prop, $cond);
  }
}

<<__EntryPoint>>
function main() {
  $o = new asshat();
  $o->run(true);
}
