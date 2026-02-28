
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reserves enough memory to accommodate a given number of elements




``` Hack
public function reserve(
  int $sz,
): void;
```




Reserves enough memory for ` $sz ` elements. If `` $sz `` is less than or
equal to the current capacity of the current [` Vector `](/apis/Classes/HH/Vector/), this method does
nothing.




If ` $sz ` is less than zero, an exception is thrown.




## Parameters




+ ` int $sz ` - The pre-determined size you want for the current [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* ` void `




## Examples




This example reserves space for 1000 elements and then fills the [` Vector `](/apis/Classes/HH/Vector/) with 1000 integers:




``` basic-usage.hack no-auto-output
const int VECTOR_SIZE = 1000;

<<__EntryPoint>>
function basic_usage_main(): void {
  $v = Vector {};
  $v->reserve(VECTOR_SIZE);

  for ($i = 0; $i < VECTOR_SIZE; $i++) {
    $v[] = $i * 10;
  }

  \var_dump($v);
}
```
<!-- HHAPIDOC -->
