
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




Reserves enough memory for ` sz ` elements. If `` sz `` is less than or equal
to the current capacity of this [` Set `](/apis/Classes/HH/Set/), this method does nothing.




## Parameters




+ ` int $sz ` - The pre-determined size you want for the current [` Set `](/apis/Classes/HH/Set/).




## Returns




* ` void `




## Examples




This example reserves space for 1000 elements and then fills the [` Set `](/apis/Classes/HH/Set/) with 1000 integers:




``` basic-usage.hack no-auto-output
const int SET_SIZE = 1000;

<<__EntryPoint>>
function basic_usage_main(): void {
  $s = Set {};
  $s->reserve(SET_SIZE);

  for ($i = 0; $i < SET_SIZE; $i++) {
    $s[] = $i * 10;
  }

  \var_dump($s);
}
```
<!-- HHAPIDOC -->
