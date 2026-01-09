
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Same as dynamic_class_meth but can't be used in RepoAuthoritative mode
and doesn't raise warnings or errors
on methods not marked __DynamicallyCallable




``` Hack
namespace HH;

function dynamic_class_meth_force(
  string $cls,
  string $meth,
): mixed;
```




## Parameters




+ ` string $cls `
+ ` string $meth `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
