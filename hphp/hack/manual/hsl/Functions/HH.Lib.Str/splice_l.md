
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the string with a slice specified by the offset/length replaced by the
given replacement string




``` Hack
namespace HH\Lib\Str;

function splice_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  string $replacement,
  int $offset,
  ?int $length = NULL,
): string;
```




If the length is omitted or exceeds the upper bound of the string, the
remainder of the string will be replaced. If the length is zero, the
replacement will be inserted at the offset.







Offset can be positive or negative. When positive, replacement starts from the
beginning of the string; when negative, replacement starts from the end of the string.




Some examples:

+ ` Str\splice_l($l, "apple", "orange", 0) ` without `` $length ``, ``` $string ``` is replaced, resolving to ```` "orange" ````
+ ` Str\splice_l($l, "apple", "orange", 3) ` inserting at `` $offset `` ``` 3 ``` from the start of ```` $string ```` resolves to ````` "apporange" `````
+ ` Str\splice_l($l, "apple", "orange", -2) ` inserting at `` $offset `` ``` -2 ``` from the end of ```` $string ```` resolves to ````` "apporange" `````
+ ` Str\splice_l($l, "apple", "orange", 0, 0) ` with `` $length `` ``` 0 ```, ```` $replacement ```` is appended at ````` $offset ````` `````` 0 `````` and resolves to ``````` "orangeapple" ```````
+ ` Str\splice_l($l, "apple", "orange", 5, 0) ` with `` $length `` ``` 0 ```, ```` $replacement ```` is appended at ````` $offset ````` `````` 5 `````` and resolves to ``````` "appleorange" ```````




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $string `
- ` string $replacement `
- ` int $offset `
- ` ?int $length = NULL `




## Returns




+ ` string `




## Examples




``` basic-usage.hack
$locale = \HH\Lib\Locale\create("en_US.UTF-8");
$result = Str\splice_l($locale, "apple", "orange", 5, 0);
echo "Splice string with en_US.UTF-8: $result \n";
```
<!-- HHAPIDOC -->
