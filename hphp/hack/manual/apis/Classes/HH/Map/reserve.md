
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




Reserves enough memory for ` sz ` elements. If `` sz `` is less than or
equal to the current capacity of this [` Map `](/apis/Classes/HH/Map/), this method does nothing.




## Parameters




+ ` int $sz ` - The pre-determined size you want for the current [` Map `](/apis/Classes/HH/Map/).




## Returns




* ` void `




## Examples




This example reserves space for 1000 elements and then fills the [` Map `](/apis/Classes/HH/Map/) with 1000 integer keys and values:




``` basic-usage.hack no-auto-output
const int MAP_SIZE = 1000;

<<__EntryPoint>>
function basic_usage_main(): void {
  $m = Map {};
  $m->reserve(MAP_SIZE);

  for ($i = 0; $i < MAP_SIZE; $i++) {
    $m[$i] = $i * 10;
  }

  \var_dump($m);
}
```
<!-- HHAPIDOC -->
