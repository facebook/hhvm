
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Modify a value







``` Hack
public function prepend(
  string $key,
  string $value,
): Awaitable<void>;
```




## Parameters




+ ` string $key ` - Name of the key to modify
+ ` string $value ` - String to prepend




## Returns




* [` Awaitable<void> `](/apis/Classes/HH/Awaitable/)




## Examples




The following example shows how to use the [` MCRouter::prepend `](/apis/Classes/MCRouter/prepend/) function.




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

async function prepend_to_value(
  \MCRouter $mc,
  string $key,
  string $prepend_str,
): Awaitable<void> {
  await $mc->prepend($key, $prepend_str);
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mc = get_simple_mcrouter();
  $unique_key = \str_shuffle('ABCDEFGHIJKLMN');
  await set_value($mc, $unique_key, 'Hi');
  $val = await $mc->get($unique_key);
  \var_dump($val);
  try {
    await prepend_to_value($mc, $unique_key, 'Oh');
    $val = await $mc->get($unique_key);
    \var_dump($val);
  } catch (\MCRouterException $ex) {
    \var_dump($ex->getMessage());
  }
}
```.hhvm.expect
string(2) "Hi"
string(4) "OhHi"
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
