<?hh

async function foo(): Awaitable<void> {
  $x = (await genx()) ? 1 : 2;
  $x = (await genx())->foo(await geny());
  $x = (await genx())?->foo(42);
  $x = (int)(await genx());
  $x = (await genx())->foo;
  $x = Foo::FOO;
  $x = Foo::foo(await geny());
  $x = (await genx()) instanceof (await geny());
  $x = (await genx()) is int;
  $x = (await genx()) as int;
  $x = (await genx()) ?as int;
  $x = empty(await genx());
  $x = isset(array()[await genx()]);
  $x = Map { (await genx()) => (await geny()) };
  $x = Vector { (await genx()) };
  $x = Set { (await genx()) };
  $x = new (await genx())(await geny());
  $x = shape('key' => (await genx()));
  $x = tuple((await genx()), (await geny()));
  $x = array((await genx()), (await geny()) => (await genz()));
  $x = [(await genx()), (await geny()) => (await genz())];
  $x = darray[(await genx()) => (await geny())];
  $x = varray[(await genx()), (await geny())];
  $x = dict[(await genx()) => (await geny())];
  $x = vec[(await genx()), (await geny())];
  $x = keyset[(await genx()), (await geny())];
  $x = (await genx())[(await geny())];
  $x = ((await genx()) and true);
  $x = ((await genx()) or true);
  $x = (await genx()) && true;
  $x = (await genx()) || true;
  $x = (await genx()) ?: 42;
  $x = (await genx()) ?? 42;
  $x += (await genx());
  $x -= (await genx());
  $x *= (await genx());
  $x /= (await genx());
  $x **= (await genx());
  $x .= (await genx());
  $x %= (await genx());
  $x ^= (await genx());
  $x |= (await genx());
  $x &= (await genx());
  $x <<= (await genx());
  $x >>= (await genx());

  $x = (await genx()) + (await geny());
  $x = (await genx()) - (await geny());
  $x = (await genx()) * (await geny());
  $x = (await genx()) / (await geny());
  $x = (await genx()) ** (await geny());
  $x = (await genx()) === (await geny());
  $x = (await genx()) == (await geny());
  $x = (await genx()) <= (await geny());
  $x = (await genx()) >= (await geny());
  $x = (await genx()) < (await geny());
  $x = (await genx()) > (await geny());
  $x = (await genx()) <> (await geny());
  $x = (await genx()) <=> (await geny());
  $x = (await genx()) != (await geny());
  $x = (await genx()) !== (await geny());
  $x = (await genx()) & (await geny());
  $x = (await genx()) | (await geny());
  $x = (await genx()) << (await geny());
  $x = (await genx()) >> (await geny());
  $x = (await genx()) % (await geny());
  $x = (await genx()) ^ (await geny());
  $x = (await genx()) |> f($$);
  $x = 42 |> f(await $$);
  $x = (await genx()) |> (await f(42)) + $$;

  +(await genx());
  -(await genx());
  !(await genx());
  ~(await genx());
  @foo(await genx());
  clone (await genx());
  print (await genx());

  await genx();
  return await genx();
  unset(array()[await genx()]);
  if (await genx()) {}
  throw (await genx());
  switch (await genx()) {}
  foreach ((await genx()) as $k => $v) {}
  foreach ((await genx()) await as $k => $v) {}
  for ((await genx()); 42; 43) {}
  try { await genx(); } catch (Exception $_) {} finally {}
  try {} catch (Exception $_) { await geny(); } finally {}
  try {} catch (Exception $_) {} finally { await genz(); }
  echo (await genx());
  do { await genx(); } while (false);
}

async function bar() {
  yield (await genx());
}
