
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Asynchronously wait for the condition variable to be notified and
return the result or throw the exception received via notification




``` Hack
final public function waitForNotificationAsync(
  Awaitable<void> $notifiers,
): Awaitable<T>;
```




The caller must provide an Awaitable $notifiers (which must be a
WaitHandle) that must not finish before the notification is received.
This means $notifiers must represent work that is guaranteed to
eventually trigger the notification. As long as the notification is
issued only once, asynchronous execution unrelated to $notifiers is
allowed to trigger the notification.




## Parameters




+ [` Awaitable<void> `](/apis/Classes/HH/Awaitable/)`` $notifiers ``




## Returns




* [` Awaitable<T> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
