
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after a
mapping operation has been applied to each key/value pair in the provided
[` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function vmkw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function(Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<Tr>>>;
```




This function is similar to ` vmk() `, except the [` Vector `](/apis/Classes/HH/Vector/) in the returned
[` Awaitable `](/apis/Classes/HH/Awaitable/) contains `` ResultOrExceptionWrapper ``s instead of raw values.




This function is similar to [` Vector::mapWithKey() `](/apis/Classes/HH/Vector/mapWithKey/), but the mapping of the
key/value pairs are done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vmkw ` because we are returning a `` v ``ector, doing a
``` m ```apping operation that includes both ```` k ````eys and values, and each member
of the [` Vector `](/apis/Classes/HH/Vector/) is `` w ``rapped by a ``` ResultOrExceptionWrapper ```.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/).




The ` ResultOrExceptionWrapper `s in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of keys and values to map.
+ ` (function(Tk, Tv): Awaitable<Tr>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<ResultOrExceptionWrapper<Tr>>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper `` after the
  mapping operation has been applied to the keys and values in
  ``` $inputs ```.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  // Map a vector of numbers to their value divided by their index
  // throwing on division by zero.
  $quotients = await \HH\Asio\vmkw(
    Vector {1, 2, 6, 12},

    async ($idx, $val) ==> {
      if ($idx != 0) {
        return $val / $idx;
      } else {
        throw new \Exception(
          "Division by zero: ".\print_r($val, true).'/'.\print_r($idx, true),
        );
      }
    },
  );

  foreach ($quotients as $result) {
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
