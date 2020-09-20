<?hh

class C<<<__Warn>> reify T> {}
class D {}

async function f(mixed $x): Awaitable<C<int>> {
  return $x;
}
<<__EntryPoint>> function main(): void {
\HH\Asio\join(f(new C<int>()));    // correct
\HH\Asio\join(f(new C<string>())); // warn
\HH\Asio\join(f(new D()));         // error
}
