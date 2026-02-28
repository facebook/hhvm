
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec containing the string split on the given delimiter




``` Hack
namespace HH\Lib\Str;

function split_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  string $delimiter,
  ?int $limit = NULL,
): vec<string>;
```




The vec
will not contain the delimiter itself.




If the limit is provided, the vec will only contain that many elements, where
the last element is the remainder of the string.




To split the string into equally-sized chunks, see ` Str\chunk_l() `.
To use a pattern as delimiter, see `` Regex\split() ``.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` string $delimiter `
* ` ?int $limit = NULL `




## Returns




- ` vec<string> `
<!-- HHAPIDOC -->
