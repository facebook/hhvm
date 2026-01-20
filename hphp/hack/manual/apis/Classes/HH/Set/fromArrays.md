
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing all the values from the specified `` array ``(s)




``` Hack
public static function fromArrays(
  ...$argv,
): Set<Tv>;
```




## Parameters




+ ` ...$argv ` - The `` array ``s to convert to a [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) with the values from the passed `` array ``(s).




## Examples




This example shows that duplicate values in the input arrays only appear once in the final [` Set `](/apis/Classes/HH/Set/):




``` basic-usage.hack
$s = Set::fromArrays(
  varray['red'],
  varray['green', 'blue'],
  varray['yellow', 'red'], // Duplicate 'red' will be ignored
);

\var_dump($s);
```
<!-- HHAPIDOC -->
