<?hh

class C<reify T> {}

async function f(mixed $x): Awaitable<?C<int>> {
  return $x;
}
<<__EntryPoint>> function main(): void {
\HH\Asio\join(f(new C<int>()));
\HH\Asio\join(f(null));

echo "done\n";
}
