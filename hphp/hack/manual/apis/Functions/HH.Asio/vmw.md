
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after a
mapping operation has been applied to each value in the provided
[` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
namespace HH\Asio;

function vmw<Tv, Tr>(
  Traversable<Tv> $inputs,
  (function(Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<Tr>>>;
```




This function is similar to ` vm() `, except the [` Vector `](/apis/Classes/HH/Vector/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains `` ResultOrExceptionWrapper ``s instead of raw values.




This function is similar to [` Vector::map() `](/apis/Classes/HH/Vector/map/), but the mapping of the values
is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vmw ` because we are returning a `` v ``ector, doing a
``` m ```apping operation and each member of the [` Vector `](/apis/Classes/HH/Vector/) is `` w ``rapped by a
``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The ` ResultOrExceptionWrapper `s in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $inputs `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) of values to map.
+ ` (function(Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<ResultOrExceptionWrapper<Tr>>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after the
  mapping operation has been applied to the values in ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Map a vector of numbers to half integer half
  // throwing if they can't be divided evenly
  $halves = await \HH\Asio\vmw(
    Vector {1, 2, 3, 4},

    async ($val) ==> {
      if ($val % 2) {
        throw new \Exception("$val is an odd number");
      } else {
        return $val / 2;
      }
    },
  );

  foreach ($halves as $result) {
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
