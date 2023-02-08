<?hh

function test(bool $b): void {
  $c = $b ? Vector<int>{} : Set<int> {};
  $f = async function ($_): Awaitable<int> { return 42; };
  $c->map<int>($f);
  $c->map<int>(async function ($_) : Awaitable<int> { return 42; });
}
