
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Does the PHP style behaviour when doing == or ===




``` Hack
namespace HH\Lib\Legacy_FIXME;

function eq(
  mixed $l,
  mixed $r,
): bool;
```




tl/dr this involves a lot of potential implicit coercions. see
__cast_and_compare for the complete picture.




## Parameters




+ ` mixed $l `
+ ` mixed $r `




## Returns




* ` bool `
<!-- HHAPIDOC -->
