
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
public function getKey(): string;
```




## Returns




+ ` string `




## Examples




The following example shows how to retrieve the key from an [` MCRouterException `](/apis/Classes/MCRouterException/) using its `` getKey `` method. If there is no key associated with the exception, then ``` "" ``` will be returned.




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
  // can also pass optional int flags and int expiration time (in seconds)
  await $mc->add($key, $value);
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mc = get_simple_mcrouter();
  $unique_key = \str_shuffle('ABCDEFGHIJKLMN');
  await add_value($mc, $unique_key, "Hi");
  $val = await $mc->get($unique_key);
  try {
    // Shouldn't be able to add the same key twice
    await add_value($mc, $unique_key, "Bye");
  } catch (\MCRouterException $ex) {
    \var_dump($ex->getKey()); // will output $unique_key
  }
}
```.hhvm.expectf
string(%d) "%s"
```.example.hhvm.out
string(14) "LGMEBIKHADCFJN"
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
