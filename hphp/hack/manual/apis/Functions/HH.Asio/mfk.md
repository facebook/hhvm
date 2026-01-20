
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after a filtering operation has been
applied to each key and value in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mfk<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tk, Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>>;
```




This function is similar to ` mf() `, but passes element keys to the callable
as well.




This function is similar to [` Map::filterWithKey() `](/apis/Classes/HH/Map/filterWithKey/), but the filtering of the
keys and values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mfk ` because we are returning a `` m ``ap, doing a
a ``` f ```iltering operation that includes ```` k ````eys.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The keys and values in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not
available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to filter.
+ ` (function(Tk, Tv): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after the filtering operation has been
  applied to both the keys and values in `` $inputs ``.




## Examples




``` basic-usage.hack
enum COLOR: int {
  RED = 1;
  ORANGE = 2;
  YELLOW = 3;
  GREEN = 4;
  BLUE = 5;
  INDIGO = 6;
  VIOLET = 7;
}

<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $fruits = ImmMap {
    'Apple' => COLOR::RED,
    'Banana' => COLOR::YELLOW,
    'Grape' => COLOR::GREEN,
    'Orange' => COLOR::ORANGE,
    'Pineapple' => COLOR::YELLOW,
    'Tangerine' => COLOR::ORANGE,
  };

  // Similar to $times->filter(...)
  // But awaits the awaitable result of the callback
  // rather than using it directly
  $not_self_named = await \HH\Asio\mfk(
    $fruits,

    // Exclude fruits who's name is the same as their color
    async ($name, $color) ==> \strcasecmp($name, COLOR::getNames()[$color]),
  );

  foreach ($not_self_named as $fruit => $color) {
    echo $fruit, 's are ', COLOR::getNames()[$color], "\n";
  }
}
```
<!-- HHAPIDOC -->
