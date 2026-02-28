
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of `` ResultOrExceptionWrapper `` after a
mapping operation has been applied to each value in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mmw<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>>;
```




This function is similar to ` mm() `, except the [` Map `](/apis/Classes/HH/Map/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains values of `` ResultOrExceptionWrapper `` instead of raw
values.




This function is similar to [` Map::map() `](/apis/Classes/HH/Map/map/), but the mapping of the values
is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mmw ` because we are returning a `` m ``ap, doing a
``` m ```apping operation and each value member in the [` Map `](/apis/Classes/HH/Map/) is `` w ``rapped by a
``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The ` ResultOrExceptionWrapper `s in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of values to map.
+ ` (function(Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` ResultOrExceptionWrapper<Tr>>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of key/`` ResultOrExceptionWrapper `` pairs
  after the mapping operation has been applied to the values in
  ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Map a map of numbers to their integer half
  // throwing if they can't be divided evenly
  $halves = await \HH\Asio\mmw(
    Map {
      'one' => 1,
      'two' => 2,
      'three' => 3,
      'four' => 4,
    },

    async ($val) ==> {
      if ($val % 2) {
        throw new \Exception("$val is an odd number");
      } else {
        return $val / 2;
      }
    },
  );

  foreach ($halves as $num => $result) {
    if ($result->isSucceeded()) {
      echo "$num / two Success: ";
      \var_dump($result->getResult());
    } else {
      echo "$num / two Failed: ";
      \var_dump($result->getException()->getMessage());
    }
  }
}
```
<!-- HHAPIDOC -->
