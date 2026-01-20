
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of `` ResultOrExceptionWrapper `` after a
filtering operation has been applied to each key/value pair in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mfkw<Tk as arraykey, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function(Tk, T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>>;
```




This function is similar to ` mfk() `, except the [` Map `](/apis/Classes/HH/Map/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains values of `` ResultOrExceptionWrapper `` instead of raw
values.




This function is similar to [` Map::filterWithKey() `](/apis/Classes/HH/Map/filterWithKey/), but the filtering of the
keys and values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mfkw ` because we are returning a `` m ``ap, doing a
``` f ```iltering operation on ```` k ````eys and values, and each value member in the
[` Map `](/apis/Classes/HH/Map/) is `` w ``rapped by a ``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The ` ResultOrExceptionWrapper `s in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` T> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to filter.
+ ` (function(Tk, T): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` ResultOrExceptionWrapper<T>>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of key/`` ResultOrExceptionWrapper `` pairs
  after the filtering operation has been applied to the keys an
  values in ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Return all non-negative odd numbers
  // Positive evens filtered out,
  // Negatives and zero cause exception
  $odds = await \HH\Asio\mfkw(
    Map {
      '-one' => -1,
      'zero' => 0,
      'one' => 1,
      'two' => 2,
      'three' => 3,
      'four' => 4,
    },

    async ($num, $val) ==> {
      if ($val <= 0) {
        throw new \Exception("$num is non-positive");
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
