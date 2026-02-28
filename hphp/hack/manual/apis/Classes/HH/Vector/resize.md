
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Resize the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function resize(
  int $size,
  Tv $value,
): void;
```




Resize the current [` Vector `](/apis/Classes/HH/Vector/) to contain `` $sz `` elements. If ``` $sz ``` is smaller
than the current size of the current [` Vector `](/apis/Classes/HH/Vector/), elements are removed from
the end of the current [` Vector `](/apis/Classes/HH/Vector/). If `` $sz `` is greater than the current size
of the current [` Vector `](/apis/Classes/HH/Vector/), the current [` Vector `](/apis/Classes/HH/Vector/) is extended by appending as
many copies of `` $value `` as needed to reach a size of ``` $sz ``` elements.




` $value ` can be `` null ``.




If ` $sz ` is less than zero, an exception is thrown.




## Parameters




+ ` int $size `
+ ` Tv $value ` - The value to use as the filler if we are increasing the
  size of the current [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* ` void `




## Examples




This example shows how ` resize ` can be used to decrease and increase the size of a [` Vector `](/apis/Classes/HH/Vector/):




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Resize the Vector to 2 (removing 'blue' and 'yellow')
$v->resize(2, null);
\var_dump($v);

// Resize the Vector back to 4 (filling in 'unknown' for new elements)
$v->resize(4, 'unknown');
\var_dump($v);
```
<!-- HHAPIDOC -->
