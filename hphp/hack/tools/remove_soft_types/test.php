<?hh


class C {
  public static function f(
    @array<int, string> $foo,
    $whatever
  ): @void {}
}

function g(@string $bar): @void {}

function h(): @Vector<int> {
}

class :foo:bar:hello-world {
  public function i(@int $x) {
  }
}

C::f(null);
g(null);
$f = <foo:bar:hello-world>toto</foo:bar:hello-world>;
$f->i("titi");

function one(): @int {
  return 'nope';
}

async function two(): @Awaitable<int> {
  return 'nope';
}

class D {
  public function one(): @int {
    return 'nope';
  }

  public async function two(): @Awaitable<int> {
    return 'nope';
  }
}

one();
two()->getWaitHandle()->join();
$d = new D();
$d->one();
$d->two()->getWaitHandle()->join();
