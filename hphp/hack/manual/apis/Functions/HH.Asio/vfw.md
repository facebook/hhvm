
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after a
filtering operation has been applied to each value in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function vfw<Tk, T>(
  KeyedTraversable<Tk, T, mixed> $inputs,
  (function(T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>>;
```




This function is similar to ` vf() `, except the [` Vector `](/apis/Classes/HH/Vector/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains `` ResultOrExceptionWrapper ``s instead of raw values.




This function is similar to [` Vector::filter() `](/apis/Classes/HH/Vector/filter/), but the mapping of the values
is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vfw ` because we are returning a `` v ``ector, doing a
``` f ```iltering operation and each member of the [` Vector `](/apis/Classes/HH/Vector/) is `` w ``rapped by a
``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The ` ResultOrExceptionWrapper `s in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` T, mixed> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of values to map.
+ ` (function(T): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<ResultOrExceptionWrapper<T>>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after the
  filtering operation has been applied to the values in ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Return all non-negative odd numbers
  // Positive evens filtered out,
  // Negatives and zero cause exception
  $odds = await \HH\Asio\vfw(
    Vector {-1, 0, 1, 2, 3, 4},

    async ($val) ==> {
      if ($val <= 0) {
        throw new \Exception("$val is non-positive");
      } else {
        return ($val % 2) == 1;
      }
    },
  );

  foreach ($odds as $result) {
    if ($result->isSucceeded()) {
      echo "Success: ";
      \var_dump($result->getResult());
    } else {
      echo "Failed: ";
      \var_dump($result->getException()->getMessage());
    }
  }
}
```
<!-- HHAPIDOC -->
