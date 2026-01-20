
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the remote server's current version







``` Hack
public function version(): Awaitable<string>;
```




## Returns




+ ` string ` - - The remote version




## Examples




The following example allows you to use [` MCRouter::version `](/apis/Classes/MCRouter/version/) to get the version information of the memcached server you are connected to.




~~~ basic-usage.hack
function get_simple_mcrouter(): \MCRouter {
  $servers = Vector {\getenv('HHVM_TEST_MCROUTER')};
  $mc = \MCRouter::createSimple($servers);
  return $mc;
}

async function get_version(\MCRouter $mc): Awaitable<?string> {
  $ret = null;
  try {
    $ret = await $mc->version();
  } catch (\MCRouterException $ex) {
    \var_dump($ex->getMessage());
  }
  return $ret;

}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mc = get_simple_mcrouter();
  $ver = await get_version($mc);
  \var_dump($ver);
}
```.hhvm.expectf
string(%d) "%s"
```.example.hhvm.out
string(5) "1.4.4"
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
