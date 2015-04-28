<?hh

function block() { // simulates blocking I/O
  return RescheduleWaitHandle::create(1,1);
};

class CWithClosures {
  public function __construct(private $a) {}

  public function test($a) {
    $f = function (...$args) use ($a) {
      echo __METHOD__, "\n";
      var_dump($args);
      var_dump($a);
      var_dump($this->a);
      $this->variadic($a, $this->a, reset($args));
    };
    $f('a', 'b');
  }

  private function variadic(...$args) {
    echo __METHOD__, "\n";
    var_dump($args);
  }
}

function main() {
  echo 'basic closure', "\n";
  $f = function (...$args) {
    var_dump($args);
  };
  $f('a', 'b');

  echo 'eager async closure', "\n";
  $f = async function (...$args) {
    var_dump($args);
  };
  HH\Asio\join($f('a', 'b'));

  echo 'blocking async closure', "\n";
  $f = async function (...$args) {
    await block();
    var_dump($args);
  };
  HH\Asio\join($f('a', 'b'));

  $c = new CWithClosures('prop-a');
  $c->test('a');
}
main();
