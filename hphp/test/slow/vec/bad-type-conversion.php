<?hh

abstract final class FooStatics {
  public static $x = false;
}

async function foo() {
  FooStatics::$x = !FooStatics::$x;
  return FooStatics::$x;
}

class X {
  public async function f() {
    $buckets = vec[];

    $foo = await foo();
    if ($foo) {
      $buckets[] = "hello";
    }

    return $this->g($buckets);
  }

  public function g(vec $v) { return $v; }
}

function test() {
  $x = new X;
  var_dump(HH\Asio\join($x->f()));
  var_dump(HH\Asio\join($x->f()));
}


<<__EntryPoint>>
function main_bad_type_conversion() {
test();
}
