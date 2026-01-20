
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Atomicly increment a numeric value







``` Hack
public function incr(
  string $key,
  int $val,
): Awaitable<int>;
```




## Parameters




+ ` string $key ` - Name of the key to modify
+ ` int $val ` - Amount to increment




## Returns




* ` int ` - - The new value




## Examples




The following example shows how to increment a value of a key by a specified integer using [` MCRouter::incr `](/apis/Classes/MCRouter/incr/). The value **must** be numeric.




~~~ basic-usage.hack
function get_simple_mcrouter(): \MCRouter {
  $servers = Vector {\getenv('HHVM_TEST_MCROUTER')};
  $mc = \MCRouter::createSimple($servers);
  return $mc;
}

async function set_value(
  \MCRouter $mc,
  string $key,
  string $value,
): Awaitable<void> {
  // can also pass optional int flags and int expiration time (in seconds)
  await $mc->set($key, $value);
}

async function inc_value(
  \MCRouter $mc,
  string $key,
  int $amount,
): Awaitable<void> {
  await $mc->incr($key, $amount);
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mc = get_simple_mcrouter();
  $unique_key = \str_shuffle('ABCDEFGHIJKLMN');
  await set_value($mc, $unique_key, "1");
  $val = await $mc->get($unique_key);
  \var_dump($val);
  await inc_value($mc, $unique_key, 3);
  $val = await $mc->get($unique_key);
  \var_dump($val);

  // Try on a value not numeric
  $unique_key = \str_shuffle('ABCDEFGHIJKLMN');
  await set_value($mc, $unique_key, "B");
  $val = await $mc->get($unique_key);
  \var_dump($val);
  try {
    await inc_value($mc, $unique_key, 3); // won't be "E" :)
    $val = await $mc->get($unique_key);
    \var_dump($val);
  } catch (\MCRouterException $ex) {
    \var_dump($ex->getMessage()); // can't increment on a string
    \var_dump($val);
  }
}
```.hhvm.expectf
string(1) "1"
string(1) "4"
string(1) "B"
string(39) "incr failed with result mc_res_notfound"
string(1) "B"
```.example.hhvm.out
string(1) "1"
string(1) "4"
string(1) "B"
string(39) "incr failed with result mc_res_notfound"
string(1) "B"
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
