
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Fix invalid inputs to ` Str\replace_every ` and similar functions




``` Hack
namespace HH\Lib\Legacy_FIXME;

function coerce_possibly_invalid_str_replace_pairs(
  KeyedContainer<string, string> $pairs,
): dict<string, string>;
```




Replacement pairs are required to be a string-to-string map, where the key
is a non-empty string (as find-replace for the empty string doesn't make
sense).




Previously, these requirements were not consistently enforced; the HSL would
sometimes raise an error, but sometimes would coerce to string, and silently
drop empty string keys.




Non-string keys/values required a FIXME.




This function is intended to be used like so:




$out = Str\\replace_every(
$in,
Legacy_FIXME\\coerce_possibly_invalid_str_replace_pairs($replacements)
);




Calls to this function should be removed when safe to do so.




## Parameters




+ [` KeyedContainer<string, `](/apis/Interfaces/HH/KeyedContainer/)`` string> $pairs ``




## Returns




* ` dict<string, string> `
<!-- HHAPIDOC -->
