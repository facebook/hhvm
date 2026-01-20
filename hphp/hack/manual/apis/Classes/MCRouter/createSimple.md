
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Simplified constructor







``` Hack
public static function createSimple(
  Vector $servers,
): MCRouter;
```




## Parameters




+ [` Vector `](/apis/Classes/HH/Vector/)`` $servers `` - List of memcache servers to connect to




## Returns




* [` MCRouter `](/apis/Classes/MCRouter/) - Instance of MCRouter




## Examples




The following example shows you how use [` MCRouter::createSimple `](/apis/Classes/MCRouter/createSimple/) to create an instance of [` MCRouter `](/apis/Classes/MCRouter/). You only need to pass it a [` Vector `](/apis/Classes/HH/Vector/) containing one or more locations of Memcached servers; default configurations are used after that (e.g, `` route = 'PoolRoute|P' ``).




~~~ basic-usage.hack
$servers = Vector {\getenv('HHVM_TEST_MCROUTER')};
$mc = \MCRouter::createSimple($servers);
\var_dump($mc is \MCRouter);
```.hhvm.expectf
bool(true)
```.example.hhvm.out
bool(true)
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
