
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` Str\split() `, with old behavior for negative limits




``` Hack
namespace HH\Lib\Legacy_FIXME;

function split_with_possibly_negative_limit(
  string $string,
  string $delimiter,
  ?int $limit = NULL,
): vec<string>;
```




` Str\split() ` now consistently bans negative limits.




Previously, negative limits were banned if the delimiter were the empty
string, but other delimiters would lead to truncation - unlike positive
limits, which lead to concatenation.




For example:




Str\\split('a!b!c', '!') === vec['a', 'b', c']
Str\\split('a!b!c', '!', 2) === vec['a', 'b!c']
Str\\split('a!b!c', '!', -1) === vec['a', 'b']







This function reimplements this old behavior; ` Str\split() ` will now
consistently throw on negative limits.




## Parameters




+ ` string $string `
+ ` string $delimiter `
+ ` ?int $limit = NULL `




## Returns




* ` vec<string> `
<!-- HHAPIDOC -->
