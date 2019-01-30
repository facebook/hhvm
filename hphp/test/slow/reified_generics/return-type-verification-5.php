<?hh

class C<reify T> {}

async function f(mixed $x): Awaitable<?C<int>> {
  return $x;
}

\HH\Asio\join(f(new C<reify int>()));
\HH\Asio\join(f(null));

echo "done\n";
