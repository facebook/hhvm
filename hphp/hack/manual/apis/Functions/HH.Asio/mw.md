
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Translate a [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) of `` Awaitables `` into a single [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of
[` Map `](/docs/apis/Classes/HH/Map/) of key/`` ResultOrExceptionWrapper `` pairs




``` Hack
namespace HH\Asio;

function mw<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tv>>>;
```




This function is the same as ` m() `, but wraps the results into
key/`` ResultOrExceptionWrapper `` pairs.




This function takes any [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) object of `` Awaitables `` (i.e., each
member of the [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) has a value of type [` Awaitable `](/docs/apis/Classes/HH/Awaitable/), likely
from a call to a function that returned [` Awaitable<T> `](/docs/apis/Classes/HH/Awaitable/)), and transforms those
`` Awaitables `` into one big [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) [` Map `](/docs/apis/Classes/HH/Map/) of
key/`` ResultOrExceptionWrapper `` pairs.




This function is called ` mw ` because we are returning a `` m ``ap of
[` Awaitable `](/docs/apis/Classes/HH/Awaitable/) `` w ``rapped into ``` ResultofExceptionWrapper ```s.




The ` ResultOrExceptionWrapper ` values in the [` Map `](/docs/apis/Classes/HH/Map/) of the returned
[` Awaitable `](/docs/apis/Classes/HH/Awaitable/) are not available until you `` await `` or ``` join ``` the returned
[` Awaitable `](/docs/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/docs/apis/Interfaces/HH/KeyedTraversable/)`` Awaitable<Tv>> $awaitables `` - The collection of [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) awaitables.




## Returns




* [` Awaitable<Map<Tk, `](/docs/apis/Classes/HH/Awaitable/)`` ResultOrExceptionWrapper<Tv>>> `` - An [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of [` Map `](/docs/apis/Classes/HH/Map/) of key/`` ResultOrExceptionWrapper `` pairs,
  where the [` Map `](/docs/apis/Classes/HH/Map/) was generated from each [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) member
  in `` $awaitables ``.




## Examples




~~~ basic-usage.hack
async function one(): Awaitable<int> {
  return 1;
}

<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $mcr = \MCRouter::createSimple(ImmVector {});

  $handles = \HH\Asio\mw(Map {
    // This will throw an exception, since there's no servers to speak to
    'cache' => $mcr->get("no-such-key"),

    // While this will obviously succeed
    'one' => one(),
  });

  $results = await $handles;
  foreach ($results as $key => $result) {
    if ($result->isSucceeded()) {
      echo "$key Success: ";
      \var_dump($result->getResult());
    } else {
      echo "$key Failed: ";
      \var_dump($result->getException()->getMessage());
    }
  }
}
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
