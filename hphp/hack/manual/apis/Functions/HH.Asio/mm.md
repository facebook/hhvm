
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) containing after a mapping operation has
been applied to each value in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mm<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>>;
```




This function is similar to [` Map::map() `](/apis/Classes/HH/Map/map/), but the mapping of the values
is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mm ` because we are returning a `` m ``ap, and doing a
``` m ```apping operation.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The keys and values in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not
available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of values to map.
+ ` (function(Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` Tr>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after the mapping operation has been
  applied to the values in  `` $inputs ``.




## Examples




~~~ basic-usage.hack
/**
 * Query an arbitrary number of URLs in parallel
 * returning them as a Map of string responses.
 *
 * Refer to \HH\Asio\m() for a more verbose version of this.
 */
async function get_urls(
  \ConstMap<string, string> $urls,
): Awaitable<Map<string, string>> {

  // Invoke \HH\Asio\curl_exec for each URL,
  // then await on each in parallel
  return await \HH\Asio\mm($urls, fun("\HH\Asio\curl_exec"));
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
    echo $name, ': ';
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
