
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/reflectionmethod.export.php )




``` Hack
public static function export(
  string $class,
  string $name,
  bool $return = false,
): ?string;
```




Exports a ReflectionMethod. Warning: This function is currently not
documented; only its argument list is available.




## Parameters




+ ` string $class `
+ ` string $name `
+ ` bool $return = false `




## Returns




* ` ?string ` - If the return parameter is set to TRUE, then the
  export is returned as a string, otherwise NULL is
  returned.
<!-- HHAPIDOC -->
