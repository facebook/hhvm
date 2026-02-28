---
title: AsyncGenerator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Async generators are similar to
[PHP Generators](<http://php.net/manual/en/language.generators.overview.php>),
except that we are combining async with generators




An async generator is just like a normal generator with the addition of
allowing ` await ` statements in it because getting to the next yielded value
involves getting and awaiting on an [` Awaitable `](/apis/Classes/HH/Awaitable/).




WHILE THIS CLASS EXPOSES 3 METHODS, 99.9% OF THE TIME YOU WILL NOT USE THIS
CLASS DIRECTLY. INSTEAD, YOU WILL USE ` await as ` IN THE CODE USING YOUR
ASYNC GENERATOR. PLEASE READ THE GUIDE REFERENCED IN THIS API DOCUMENTATION
FOR MORE INFORMATION. However, we document these methods for completeness in
case you have a use case for them.




There are three type parameters associated with an AsyncGenerator:

+ ` Tk `: The type of key returned by the generator
+ ` Tv `: The type of value returned by the generator
+ ` Ts `: The type that will be passed on a call to [` send() `](/apis/Classes/HH/AsyncGenerator/send/)




## Guide




* [Generators](</hack/asynchronous-operations/generators>)







## Interface Synopsis




``` Hack
namespace HH;

final class AsyncGenerator implements AsyncKeyedIterator<Tk, Tv> {...}
```




### Public Methods




- [` ->next(): Awaitable<?(Tk, Tv)> `](/apis/Classes/HH/AsyncGenerator/next/)\
  Return the [` Awaitable `](/apis/Classes/HH/Awaitable/) associated with the next key/value tuple in the
  async generator, or `` null ``
- [` ->raise(\Exception $exception): Awaitable<?(Tk, Tv)> `](/apis/Classes/HH/AsyncGenerator/raise/)\
  Raise an exception to the async generator
- [` ->send(?Ts $value): Awaitable<?(Tk, Tv)> `](/apis/Classes/HH/AsyncGenerator/send/)\
  Send a value to the async generator and resumes execution of the generator
<!-- HHAPIDOC -->
