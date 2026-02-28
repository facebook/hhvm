---
title: ConditionNode
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A linked list node storing Condition and pointer to the next node




## Interface Synopsis




``` Hack
namespace HH\Lib\Async;

final class ConditionNode extends Condition<T> {...}
```




### Public Methods




+ [` ->addNext(): ConditionNode<T> `](/hsl/Classes/HH.Lib.Async/ConditionNode/addNext/)
+ [` ->getNext(): ?ConditionNode<T> `](/hsl/Classes/HH.Lib.Async/ConditionNode/getNext/)







### Public Methods ([` HH\Lib\Async\Condition `](/hsl/Classes/HH.Lib.Async/Condition/))




* [` ->fail(\Exception $exception): void `](/hsl/Classes/HH.Lib.Async/Condition/fail/)\
  Notify the condition variable of failure and set the exception
* [` ->succeed(T $result): void `](/hsl/Classes/HH.Lib.Async/Condition/succeed/)\
  Notify the condition variable of success and set the result
* [` ->waitForNotificationAsync(Awaitable<void> $notifiers): Awaitable<T> `](/hsl/Classes/HH.Lib.Async/Condition/waitForNotificationAsync/)\
  Asynchronously wait for the condition variable to be notified and
  return the result or throw the exception received via notification
<!-- HHAPIDOC -->
