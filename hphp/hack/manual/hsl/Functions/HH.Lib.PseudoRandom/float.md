
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a pseudorandom float in the range [0.0, 1.0) (i.e. the return value
is >= 0.0 and < 1.0)




``` Hack
namespace HH\Lib\PseudoRandom;

function float(): float;
```




This is NOT suitable for cryptographic uses.




For secure random floats, see ` SecureRandom\float `.




## Returns




+ ` float `
<!-- HHAPIDOC -->
