
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after a filtering operation has been
applied to each value in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mf<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>>;
```




This function is similar to [` Map::filter() `](/apis/Classes/HH/Map/filter/), but the filtering of the
values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mf ` because we are returning a `` m ``ap, and we are
doing a ``` f ```iltering operation.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The keys and values in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not
available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of values to map.
+ ` (function(Tv): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) after the filtering operation has been
  applied to the values in  `` $inputs ``.




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
  $orange_fruits =
    await \HH\Asio\mf($fruits, async ($color) ==> ($color == COLOR::ORANGE));

  foreach ($orange_fruits as $fruit => $color) {
    echo $fruit, 's are ', COLOR::getNames()[$color], "\n";
  }
}
```
<!-- HHAPIDOC -->
