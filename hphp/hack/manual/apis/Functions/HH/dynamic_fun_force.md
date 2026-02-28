
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Same as dynamic_fun but can't be used in RepoAuthoritative mode and
doesn't raise warnings or errors
on functions not marked __DynamicallyCallable




``` Hack
namespace HH;

function dynamic_fun_force(
  string $name,
): mixed;
```




## Parameters




+ ` string $name `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
