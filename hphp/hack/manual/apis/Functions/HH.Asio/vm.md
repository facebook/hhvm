
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) containing after a mapping operation has
been applied to each value in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
namespace HH\Asio;

function vm<Tv, Tr>(
  Traversable<Tv> $inputs,
  (function(Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<Tr>>;
```




This function is similar to [` Vector::map() `](/apis/Classes/HH/Vector/map/), but the mapping of the values
is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vm ` because we are returning a `` v ``ector, and
we are doing a ``` m ```apping operation.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The values in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not available
until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $inputs `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) of values to map.
+ ` (function(Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<Tr>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) after the mapping operation has been
  applied to the values in  `` $inputs ``.




## Examples




~~~ basic-usage.hack
/**
 * Query an arbitrary number of URLs in parallel
 * returning them as a Vector of string responses.
 *
 * Refer to \HH\Asio\v() for a more verbose version of this.
 */
async function get_urls(\ConstVector<string> $urls): Awaitable<Vector<string>> {

  // Invoke \HH\Asio\curl_exec for each URL,
  // then await on each in parallel
  return await \HH\Asio\vm($urls, fun("\HH\Asio\curl_exec"));
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
