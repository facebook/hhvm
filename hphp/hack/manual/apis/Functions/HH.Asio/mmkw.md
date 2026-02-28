
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of `` ResultOrExceptionWrapper `` after a
mapping operation has been applied to each key/value pair in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function mmkw<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>>;
```




This function is similar to ` mmk() `, except the [` Map `](/apis/Classes/HH/Map/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains values of `` ResultOrExceptionWrapper `` instead of raw
values.




This function is similar to [` Map::mapWithKey() `](/apis/Classes/HH/Map/mapWithKey/), but the mapping of the keys
and values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` mmkw ` because we are returning a `` m ``ap, doing a
``` m ```apping operation on ```` k ````eys and values, and each value member in the [` Map `](/apis/Classes/HH/Map/)
is `` w ``rapped by a ``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The ` ResultOrExceptionWrapper `s in the [` Map `](/apis/Classes/HH/Map/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to map.
+ ` (function(Tk, Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Map<Tk, `](/apis/Classes/HH/Awaitable/)`` ResultOrExceptionWrapper<Tr>>> `` - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Map `](/apis/Classes/HH/Map/) of key/`` ResultOrExceptionWrapper `` pairs
  after the mapping operation has been applied to the keys an values
  in ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Map a vector of numbers to their value divided by their index
  // throwing on division by zero.
  $quotients = await \HH\Asio\mmkw(
    Map {
      1 => 1,
      0 => 2,
      2 => 6,
      3 => 12,
    },

    async ($div, $val) ==> {
      if ($div != 0) {
        return $val / $div;
      } else {
        throw new \Exception(
          "Division by zero: ".\print_r($val, true).'/'.\print_r($div, true),
        );
      }
    },
  );

  foreach ($quotients as $result) {
    if ($result->isSucceeded()) {
      echo " Success: ";
      \var_dump($result->getResult());
    } else {
      echo "Failed: ";
      \var_dump($result->getException()->getMessage());
    }
  }
}
```
<!-- HHAPIDOC -->
