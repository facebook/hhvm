
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the underlying type of this ReflectionClass, given that it
represents an enum




``` Hack
public function getEnumUnderlyingType(): string;
```




If it does not, it throws.




## Returns




+ ` string ` - the string representation of the underlying type.
<!-- HHAPIDOC -->
