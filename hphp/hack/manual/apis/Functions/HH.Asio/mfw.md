
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of `` ResultOrExceptionWrapper `` after a
filtering operation has been applied to each value in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mfw<Tk as arraykey, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function(T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>>;
```




This function is similar to ` mf() `, except the [` Map `](/apis/Classes/HH/Map/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains values of `` ResultOrExceptionWrapper `` instead of raw
values.




This function is similar to [` Map::filter() `](/apis/Classes/HH/Map/filter/), but the filtering of the values
is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mfw ` because we are returning a `` m ``ap, doing a
``` f ```iltering operation and each value member in the [` Map `](/apis/Classes/HH/Map/) is `` w ``rapped by a
``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The ` ResultOrExceptionWrapper `s in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` T> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of values to fitler.
+ ` (function(T): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` ResultOrExceptionWrapper<T>>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of key/`` ResultOrExceptionWrapper `` pairs
  after the filterin operation has been applied to the values in
  ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Return all non-negative odd numbers
  // Positive evens filtered out,
  // Negatives and zero cause exception
  $odds = await \HH\Asio\mfw(
    Map {
      '-one' => -1,
      'zero' => 0,
      'one' => 1,
      'two' => 2,
      'three' => 3,
      'four' => 4,
    },

    async ($val) ==> {
      if ($val <= 0) {
        throw new \Exception("$val is non-positive");
      } else {
        return ($val % 2) == 1;
      }
    },
  );

  foreach ($odds as $num => $result) {
    if ($result->isSucceeded()) {
      echo "$num Success: ";
      \var_dump($result->getResult());
    } else {
      echo "$num Failed: ";
      \var_dump($result->getException()->getMessage());
    }
  }
}
```
<!-- HHAPIDOC -->
