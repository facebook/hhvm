
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the string ends with the given suffix (case-insensitive)




``` Hack
namespace HH\Lib\Str;

function ends_with_ci_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  string $suffix,
): bool;
```




Locale-specific rules for case-insensitive comparisons will be used, and
strings will be normalized before comparing if the locale specifies an
encoding that supports multiple representations of the same characters, such
as UTF-8.




For a case-sensitive check, see ` Str\ends_with_l() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` string $suffix `




## Returns




- ` bool `
<!-- HHAPIDOC -->
