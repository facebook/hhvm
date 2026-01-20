
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

function splice(
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

+ ` Str\splice("apple", "orange", 0) ` without `` $length ``, ``` $string ``` is replaced, resolving to ```` "orange" ````
+ ` Str\splice("apple", "orange", 3) ` inserting at `` $offset `` ``` 3 ``` from the start of ```` $string ```` resolves to ````` "apporange" `````
+ ` Str\splice("apple", "orange", -2) ` inserting at `` $offset `` ``` -2 ``` from the end of ```` $string ```` resolves to ````` "apporange" `````
+ ` Str\splice("apple", "orange", 0, 0) ` with `` $length `` ``` 0 ```, ```` $replacement ```` is appended at ````` $offset ````` `````` 0 `````` and resolves to ``````` "orangeapple" ```````
+ ` Str\splice("apple", "orange", 5, 0) ` with `` $length `` ``` 0 ```, ```` $replacement ```` is appended at ````` $offset ````` `````` 5 `````` and resolves to ``````` "appleorange" ```````




Previously known in PHP as ` substr_replace `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $string `
- ` string $replacement `
- ` int $offset `
- ` ?int $length = NULL `




## Returns




+ ` string `
<!-- HHAPIDOC -->
