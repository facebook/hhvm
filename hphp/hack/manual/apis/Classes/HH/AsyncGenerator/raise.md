
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Raise an exception to the async generator




``` Hack
public function raise(
  Exception $exception,
): Awaitable<?(Tk, Tv)>;
```




You should always ` await ` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) to get the actual
key/value tuple.




If ` null ` is returned, that means you have reached the end of iteration.




You cannot call [` raise() `](/apis/Classes/HH/AsyncGenerator/raise/) without having the value returned from a previous
call to [` raise() `](/apis/Classes/HH/AsyncGenerator/raise/), [` next() `](/apis/Classes/HH/AsyncGenerator/next/), [` send() `](/apis/Classes/HH/AsyncGenerator/send/), having first `` await ``ed.




## Parameters




+ ` Exception $exception `




## Returns




* [` Awaitable<?(Tk, `](/apis/Classes/HH/Awaitable/)`` Tv)> `` - The [` Awaitable `](/apis/Classes/HH/Awaitable/) that produced the yielded key/value tuple after
  the exception is processed. What is returned is a tuple or
  `` null ``.
<!-- HHAPIDOC -->
