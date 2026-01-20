
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Store a value







``` Hack
public function replace(
  string $key,
  string $value,
  int $flags = 0,
  int $expiration = 0,
): Awaitable<void>;
```




## Parameters




+ ` string $key ` - Name of the key to store
+ ` string $value ` - Datum to store
+ ` int $flags = 0 `
+ ` int $expiration = 0 `




## Returns




* [` Awaitable<void> `](/apis/Classes/HH/Awaitable/)




## Examples




The following example shows how to use [` MCRouter::replace `](/apis/Classes/MCRouter/replace/) to replace a key/value pair in the memcached server. You can only call `` replace `` on a currently existing key (e.g., one that has been added via [` MCRouter::add `](/apis/Classes/MCRouter/add/) or [` MCRouter::set `](/apis/Classes/MCRouter/set/)). In many cases a call to `` set/set `` is the same as a call to ``` add/replace ```, accomplishing the same thing.




If you pass an expiration time for the key, that is in seconds.




And these are the bitwise or style flags that can be passed to ` replace `:




```
MC_MSG_FLAG_PHP_SERIALIZED = 0x1,
MC_MSG_FLAG_COMPRESSED = 0x2,
MC_MSG_FLAG_FB_SERIALIZED = 0x4,
MC_MSG_FLAG_FB_COMPACT_SERIALIZED = 0x8,
MC_MSG_FLAG_ASCII_INT_SERIALIZED = 0x10,
MC_MSG_FLAG_NZLIB_COMPRESSED = 0x800,
MC_MSG_FLAG_QUICKLZ_COMPRESSED = 0x2000,
MC_MSG_FLAG_SNAPPY_COMPRESSED = 0x4000,
MC_MSG_FLAG_BIG_VALUE = 0X8000,
MC_MSG_FLAG_NEGATIVE_CACHE = 0x10000,
MC_MSG_FLAG_HOT_KEY = 0x20000,
```




See the [header file with the flags](<https://github.com/facebook/mcrouter/blob/5f259ed47b52f86cad750d2343edf324e80cb397/mcrouter/lib/mc/msg.h>)




~~~ basic-usage.hack
function get_simple_mcrouter(): \MCRouter {
  $servers = Vector {\getenv('HHVM_TEST_MCROUTER')};
  $mc = \MCRouter::createSimple($servers);
  return $mc;
}

async function add_value(
  \MCRouter $mc,
  string $key,
  string $value,
): Awaitable<void> {
  await $mc->add($key, $value);
}

async function replace_value(
  \MCRouter $mc,
  string $key,
  string $value,
): Awaitable<void> {
  // can also pass optional int flags and int expiration time (in seconds)
  await $mc->replace($key, $value);
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mc = get_simple_mcrouter();
  $unique_key = \str_shuffle('ABCDEFGHIJKLMN');
  try {
    // We never added or set this key, so it can't be replaced.
    await replace_value($mc, $unique_key, "Bye");
    $val = await $mc->get($unique_key);
    \var_dump($val);
  } catch (\MCRouterException $ex) {
    \var_dump($ex->getMessage()); // We will get here
  }
  await add_value($mc, $unique_key, "Hi");
  $val = await $mc->get($unique_key);
  \var_dump($val);
  await replace_value($mc, $unique_key, "Bye");
  $val = await $mc->get($unique_key);
  \var_dump($val);
}
```.hhvm.expectf
string(43) "replace failed with result mc_res_notstored"
string(2) "Hi"
string(3) "Bye"
```.example.hhvm.out
string(43) "replace failed with result mc_res_notstored"
string(2) "Hi"
string(3) "Bye"
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
