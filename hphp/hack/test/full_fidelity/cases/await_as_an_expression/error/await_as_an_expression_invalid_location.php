<?hh

async function foo(): Awaitable<void> {
  $x = 1 ? (await genx()) : 2;
  $x = 1 ? 2 : (await genx());
  $x = $y?->foo(await genx());
  $x = (false and (await genx()));
  $x = (true or (await genx()));
  $x = (false && (await genx()));
  $x = (true || (await genx()));
  $x = true ?: (await genx());
  $x = $y ?? (await genx());
  $x[(await genx())] = 42;
  $x[(await genx())] += 42;
  $x[(await genx())] -= 42;
  $x[(await genx())] *= 42;
  $x[(await genx())] /= 42;
  $x[(await genx())] **= 42;
  $x[(await genx())] .= 42;
  $x[(await genx())] %= 42;
  $x[(await genx())] ^= 42;
  $x[(await genx())] |= 42;
  $x[(await genx())] &= 42;
  $x[(await genx())] <<= 42;
  $x[(await genx())] >>= 42;
  $x[(await genx())] ??= 42;
  $x ??= (await genx());
  $x = (await genx()) |> await f($$);
  $x = (await (await genx()));
  for ($i = 0; (await genx()); 43) {}
  for ($i = 0; 42; (await genx())) {}
  while (await genx()) {}
  do {} while (await genx());
  require (await genx());
  require_once (await genx());
  include (await genx());
  include_once (await genx());
  ++$a[(await genx())];
  --$a[(await genx())];
  foo(inout (await genx()));
  foo(& $a[(await genx())]);
}
