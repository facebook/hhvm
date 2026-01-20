
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after a mapping operation has been
applied to each key and value in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mmk<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>>;
```




This function is similar to ` mm() `, but passes element keys to the callable
as well.




This function is similar to [` Map::mapWithKey() `](/apis/Classes/HH/Map/mapWithKey/), but the mapping of the keys
and values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mmk ` because we are returning a `` m ``ap and doing a
a ``` m ```apping operation that includes ```` k ````eys.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The keys and values in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not
available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to map.
+ ` (function(Tk, Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` Tr>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after the mapping operation has been
  applied to both the keys and values in `` $inputs ``.




## Examples




~~~ basic-usage.hack
/**
 * Query an arbitrary number of URLs in parallel
 * returning them as a Map of string responses.
 */
async function get_urls(
  \ConstMap<string, string> $urls,
): Awaitable<Map<string, string>> {

  // Await on curl requests in parallel and
  // prepend the request ID index
  return await \HH\Asio\mmk(
    $urls,
    async ($name, $url) ==> {
      $content = await \HH\Asio\curl_exec($url);
      return $name." => ".$content;
    },
  );
}

<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $urls = ImmMap {
    'com' => "http://example.com",
    'net' => "http://example.net",
    'org' => "http://example.org",
  };

  $pages = await get_urls($urls);
  foreach ($pages as $page) {
    echo \substr($page, 0, 22).' ... '.\substr($page, -8);
  }
}
```.skipif
// Skip if we don't have an internet connection
if (!\get_headers("www.example.com")) {
  print "skip";
}
~~~
<!-- HHAPIDOC -->
