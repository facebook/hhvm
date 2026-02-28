
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

True iff native facts are available




``` Hack
namespace HH\Facts;

function enabled(): bool;
```




If this returns false, any other operations in the HH\\Facts namespace will
throw InvalidOperationException if called.




## Returns




+ ` bool `
<!-- HHAPIDOC -->
