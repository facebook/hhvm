
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This is an HHVM only function that gets the raw text associated with
a default parameter




``` Hack
public function getDefaultValueText();
```




For example, for:
function foo($x = FOO*FOO)




"FOO*FOO" is returned.




getDefaultValue() will return the result of FOO*FOO.




## Returns




+ ` string ` - The raw text of a default value, or empty if it does not
  exist.
<!-- HHAPIDOC -->
