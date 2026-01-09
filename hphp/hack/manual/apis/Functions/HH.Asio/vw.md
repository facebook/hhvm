
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Translate a [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) of `` Awaitables `` into a single [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of
[` Vector `](/docs/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper ``




``` Hack
namespace HH\Asio;

function vw<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<ResultOrExceptionWrapper<Tv>>>;
```




This function is the same as ` v() `, but wraps the results into
`` ResultOrExceptionWrapper ``s.




This function takes any [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) object of `` Awaitables `` (i.e., each
member of the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is of type of [` Awaitable `](/docs/apis/Classes/HH/Awaitable/), likely from a call
to a function that returned [` Awaitable<T> `](/docs/apis/Classes/HH/Awaitable/)), and transforms those
`` Awaitables `` into one big [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) [` Vector `](/docs/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper ``.




This function is called ` vw ` because we are returning a `` v ``ector of
[` Awaitable `](/docs/apis/Classes/HH/Awaitable/) `` w ``rapped into ``` ResultofExceptionWrapper ```s.




The ` ResultOrExceptionWrapper `s in the [` Vector `](/docs/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/docs/apis/Classes/HH/Awaitable/)
are not available until you `` await `` or ``` join ``` the returned [` Awaitable `](/docs/apis/Classes/HH/Awaitable/).




## Parameters




+ [` Traversable<Awaitable<Tv>> `](/docs/apis/Interfaces/HH/Traversable/)`` $awaitables `` - The collection of [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) awaitables.




## Returns




* [` Awaitable<Vector<ResultOrExceptionWrapper<Tv>>> `](/docs/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/docs/apis/Classes/HH/Awaitable/) of [` Vector `](/docs/apis/Classes/HH/Vector/) of `` ResultOrExceptionWrapper ``, where
  the [` Vector `](/docs/apis/Classes/HH/Vector/) was generated from each [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) member in
  `` $awaitables ``.




## Examples




~~~ basic-usage.hack
async function one(): Awaitable<int> {
  return 1;
}

<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $mcr = \MCRouter::createSimple(ImmVector {});

  $handles = \HH\Asio\vw(Vector {
    // This will throw an exception, since there's no servers to speak to
    $mcr->get("no-such-key"),

    // While this will obviously succeed
    one(),
  });

  $results = await $handles;
  foreach ($results as $result) {
    if ($result->isSucceeded()) {
      echo "Success: ";
      \var_dump($result->getResult());
    } else {
      echo "Failed: ";
      \var_dump($result->getException()->getMessage());
    }
  }
}
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
