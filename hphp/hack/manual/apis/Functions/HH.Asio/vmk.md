
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) after a mapping operation has been
applied to each key and value in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function vmk<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<Tr>>;
```




This function is similar to ` vm() `, but passes element keys to the callable
as well.




This function is similar to [` Vector::mapWithKey() `](/apis/Classes/HH/Vector/mapWithKey/), but the mapping of the
keys and values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vmk ` because we are returning a `` v ``ector and doing
a ``` m ```apping operation that includes ```` k ````eys.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The values in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not available
until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to map.
+ ` (function(Tk, Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<Tr>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) after the mapping operation has been
  applied to both the keys and values in `` $inputs ``.




## Examples




~~~ basic-usage.hack
/**
 * Query an arbitrary number of URLs in parallel
 * returning them as a Vector of string responses.
 */
async function get_urls(\ConstVector<string> $urls): Awaitable<Vector<string>> {

  // Await on curl requests in parallel and
  // prepend the request ID index
  return await \HH\Asio\vmk(
    $urls,
    async ($idx, $url) ==> {
      $content = await \HH\Asio\curl_exec($url);
      return $idx." => ".$content;
    },
  );
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
    echo \substr($page, 0, 20).' ... '.\substr($page, -8);
  }
}
```.skipif
// Skip if we don't have an internet connection
if (!\get_headers("www.example.com")) {
  print "skip";
}
~~~
<!-- HHAPIDOC -->
