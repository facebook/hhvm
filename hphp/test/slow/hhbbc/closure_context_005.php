<?hh

class Foo {
  private $prop = array(1,2,3);

  public function f() {
    return async function() {
      return function() {
        $this->prop = null;
      };
    };
  }

  public function getter() { return $this->prop; }
}

function main() {
  $x = new Foo;
  var_dump($x->getter());
  $z = $x->f();
  $z = HH\Asio\join($z());
  var_dump($z);
  $z();
  var_dump($x->getter());
  var_dump($z);
  var_dump($x->getter());
}

main();

