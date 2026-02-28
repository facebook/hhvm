
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Send a value to the async generator and resumes execution of the generator




``` Hack
public function send(
  ?Ts $value,
): Awaitable<?(Tk, Tv)>;
```




You should always ` await ` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) to get the actual
key/value tuple.




If ` null ` is returned, that means you have reached the end of iteration.




You cannot call [` send() `](/apis/Classes/HH/AsyncGenerator/send/) without having the value returned from a previous
call to [` send() `](/apis/Classes/HH/AsyncGenerator/send/), [` next() `](/apis/Classes/HH/AsyncGenerator/next/), [` raise() `](/apis/Classes/HH/AsyncGenerator/raise/), having first `` await ``ed.




If you pass ` null ` to [` send() `](/apis/Classes/HH/AsyncGenerator/send/), that is equivalent to calling [` next() `](/apis/Classes/HH/AsyncGenerator/next/),
but you still need an initial [` next() `](/apis/Classes/HH/AsyncGenerator/next/) call before calling [` send(null) `](/apis/Classes/HH/AsyncGenerator/send/).




## Parameters




+ ` ?Ts $value `




## Returns




* [` Awaitable<?(Tk, `](/apis/Classes/HH/Awaitable/)`` Tv)> `` - The [` Awaitable `](/apis/Classes/HH/Awaitable/) that produced the yielded key/value tuple in
  the generator. What is returned is a tuple or `` null ``.
<!-- HHAPIDOC -->
