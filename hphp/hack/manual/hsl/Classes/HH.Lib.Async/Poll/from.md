
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a Poll from the specified list of awaitables




``` Hack
public static function from(
  Traversable<Awaitable<Tv>> $awaitables,
): this;
```




See ` KeyedPoll ` if you have a [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) and want to preserve
keys.




## Parameters




+ [` Traversable<Awaitable<Tv>> `](/apis/Interfaces/HH/Traversable/)`` $awaitables ``




## Returns




* ` this `
<!-- HHAPIDOC -->
