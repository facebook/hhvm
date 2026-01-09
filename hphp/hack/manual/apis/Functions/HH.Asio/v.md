
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Translate a [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) of `` Awaitables `` into a single [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of
[` Vector `](/docs/apis/Classes/HH/Vector/)




``` Hack
namespace HH\Asio;

function v<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<Tv>>;
```




This function takes any [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) object of `` Awaitables `` (i.e., each
member of the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is of type of [` Awaitable `](/docs/apis/Classes/HH/Awaitable/), likely from a call
to a function that returned [` Awaitable<T> `](/docs/apis/Classes/HH/Awaitable/)), and transforms those
`` Awaitables `` into one big [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) [` Vector `](/docs/apis/Classes/HH/Vector/).




This function is called ` v ` we are returning a `` v ``ector of [` Awaitable `](/docs/apis/Classes/HH/Awaitable/).




Only When you ` await ` or `` join `` the resulting [` Awaitable `](/docs/apis/Classes/HH/Awaitable/), will all of the
values in the [` Vector `](/docs/apis/Classes/HH/Vector/) within the returned [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) be available.




## Parameters




+ [` Traversable<Awaitable<Tv>> `](/docs/apis/Interfaces/HH/Traversable/)`` $awaitables `` - The collection of [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) awaitables.




## Returns




* [` Awaitable<Vector<Tv>> `](/docs/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of [` Vector `](/docs/apis/Classes/HH/Vector/), where the [` Vector `](/docs/apis/Classes/HH/Vector/) was generated from
  each [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) member in `` $awaitables ``.




## Examples




~~~ basic-usage.hack
/**
 * Query an arbitrary number of URLs in parallel
 * returning them as a Vector of string responses.
 */
async function get_urls(\ConstVector<string> $urls): Awaitable<Vector<string>> {
  // Wrap each URL string into a curl_exec awaitable
  $handles = $urls->map($url ==> \HH\Asio\curl_exec($url));

  // Wait on each handle in parallel and return the results
  return await \HH\Asio\v($handles);
}

<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $urls = ImmVector {
    "http://example.com",
    "http://example.net",
    "http://example.org",
  };

  $pages = await get_urls($urls);
  foreach ($pages as $page) {
    echo \substr($page, 0, 15).' ... '.\substr($page, -8);
  }
}
```.skipif
// Skip if we don't have an internet connection
if (!\get_headers("www.example.com")) {
  print "skip";
}
~~~
<!-- HHAPIDOC -->
