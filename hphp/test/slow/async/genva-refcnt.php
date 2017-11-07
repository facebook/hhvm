<?hh

class Marker {
  public function __destruct() {
    echo "destructing\n";
  }
}

async function foo() {
  return new Marker();
}

async function bar() {
  echo "genva 1\n";
  list($a, $b) = await genva(foo(), foo());
  echo "unset a\n";
  unset($a);
  echo "unset b\n";
  unset($b);
  echo "genva 2\n";
  list(,) = await genva(foo(), foo());
  echo "genva 3\n";
  await genva(foo(), foo());
  echo "done\n";
}

\HH\Asio\join(bar());
echo "exit\n";
