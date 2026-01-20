
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the [` Awaitable `](/apis/Classes/HH/Awaitable/) associated with the next key/value tuple in the
async generator, or `` null ``




``` Hack
public function next(): Awaitable<?(Tk, Tv)>;
```




You should always ` await ` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) to get the actual
key/value tuple.




If ` null ` is returned, that means you have reached the end of iteration.




You cannot call [` next() `](/apis/Classes/HH/AsyncGenerator/next/) without having the value returned from a previous
call to [` next() `](/apis/Classes/HH/AsyncGenerator/next/), [` send() `](/apis/Classes/HH/AsyncGenerator/send/), [` raise() `](/apis/Classes/HH/AsyncGenerator/raise/), having first `` await ``ed.




## Returns




+ [` Awaitable<?(Tk, `](/apis/Classes/HH/Awaitable/)`` Tv)> `` - The [` Awaitable `](/apis/Classes/HH/Awaitable/) that produced the next key/value tuple in the
  generator. What is returned is a tuple or `` null ``.
<!-- HHAPIDOC -->
