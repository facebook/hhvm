
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after a
filtering operation has been applied to each key/value pair in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function vfkw<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function(Tk, T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>>;
```




This function is similar to ` vfk() `, except the [` Vector `](/apis/Classes/HH/Vector/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains `` ResultOrExceptionWrapper ``s instead of raw values.




This function is similar to [` Vector::filterWithKey() `](/apis/Classes/HH/Vector/filterWithKey/), but the mapping of the
key/value pairs are done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vfkw ` because we are returning a `` v ``ector, doing a
``` f ```iltering operation that includes both ```` k ````eys and values, and each member
of the [` Vector `](/apis/Classes/HH/Vector/) is `` w ``rapped by a ``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The ` ResultOrExceptionWrapper `s in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` T> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to map.
+ ` (function(Tk, T): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<ResultOrExceptionWrapper<T>>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after the
  filtering operation has been applied to the keys and values in
  ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Return all non-negative odd numbers
  // Positive evens and odds at every third index filtered out,
  // Negatives and zero cause exception
  $odds = await \HH\Asio\vfkw(
    Vector {-1, 0, 1, 2, 3, 4, 5},

    async ($idx, $val) ==> {
      if ($val <= 0) {
        throw new \Exception("$val is non-positive");
      } else {
        return ($idx % 3) && ($val % 2);
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
