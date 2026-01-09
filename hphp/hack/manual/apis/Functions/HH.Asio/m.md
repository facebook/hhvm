
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Translate a [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) of `` Awaitables `` into a single [` Awaitable `](/docs/apis/Classes/HH/Awaitable/)`` of ``Map`




``` Hack
namespace HH\Asio;

function m<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, Tv>>;
```




This function takes any [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) object of `` Awaitables `` (i.e.,
each member of the [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) has a value of type of [` Awaitable `](/docs/apis/Classes/HH/Awaitable/),
likely from a call to a function that returned [` Awaitable<T> `](/docs/apis/Classes/HH/Awaitable/)), and
transforms those `` Awaitables `` into one big [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) [` Map `](/docs/apis/Classes/HH/Map/).




This function is called ` m ` because we are returning a `` m ``ap of [` Awaitable `](/docs/apis/Classes/HH/Awaitable/).




Only When you ` await ` or `` join `` the resulting [` Awaitable `](/docs/apis/Classes/HH/Awaitable/), will all of the
key/values in the [` Map `](/docs/apis/Classes/HH/Map/) within the returned [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) be available.




## Parameters




+ [` KeyedTraversable<Tk, `](/docs/apis/Interfaces/HH/KeyedTraversable/)`` Awaitable<Tv>> $awaitables `` - The collection of [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) awaitables.




## Returns




* [` Awaitable<Map<Tk, `](/docs/apis/Classes/HH/Awaitable/)`` Tv>> `` - An [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of [` Map `](/docs/apis/Classes/HH/Map/), where the [` Map `](/docs/apis/Classes/HH/Map/) was generated from
  each [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) member in `` $awaitables ``.




## Examples




~~~ basic-usage.hack
/**
 * Query an arbitrary number of URLs in parallel
 * returning them as a Map of string responses.
 */
async function get_urls(
  \ConstMap<string, string> $urls,
): Awaitable<Map<string, string>> {
  // Wrap each URL string into a curl_exec awaitable
  $handles = $urls->map($url ==> \HH\Asio\curl_exec($url));

  // Wait on each handle in parallel and return the results
  return await \HH\Asio\m($handles);
}

<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $urls = ImmMap {
    'com' => "http://example.com",
    'net' => "http://example.net",
    'org' => "http://example.org",
  };

  $pages = await get_urls($urls);
  foreach ($pages as $name => $page) {
    echo $name.': ';
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
