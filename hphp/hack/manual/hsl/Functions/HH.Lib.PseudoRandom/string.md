
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a pseudorandom string of length ` $length `




``` Hack
namespace HH\Lib\PseudoRandom;

function string(
  int $length,
  ?string $alphabet = NULL,
): string;
```




The string is composed of
characters from ` $alphabet ` if `` $alphabet `` is specified. This is NOT suitable
for cryptographic uses.




For secure random strings, see ` SecureRandom\string `.




## Parameters




+ ` int $length `
+ ` ?string $alphabet = NULL `




## Returns




* ` string `
<!-- HHAPIDOC -->
