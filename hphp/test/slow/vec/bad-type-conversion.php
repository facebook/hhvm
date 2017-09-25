<?hh

async function foo() {
  static $x = false; $x = !$x; return $x;
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

test();
