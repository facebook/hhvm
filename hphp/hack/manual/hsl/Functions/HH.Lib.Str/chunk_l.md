
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec containing the string split into chunks with the given number
of characters




``` Hack
namespace HH\Lib\Str;

function chunk_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  int $chunk_size = 1,
): vec<string>;
```




` $chunk_size ` is in characters.




To split the string on a delimiter, see ` Str\split_l() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` int $chunk_size = 1 `




## Returns




- ` vec<string> `
<!-- HHAPIDOC -->
