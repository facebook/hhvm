
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Remove the last element of the current [` Vector `](/apis/Classes/HH/Vector/) and return it




``` Hack
public function pop(): Tv;
```




This function throws an exception if the current [` Vector `](/apis/Classes/HH/Vector/) is empty.




The current [` Vector `](/apis/Classes/HH/Vector/) will have `` n - 1 `` elements after this operation, where
``` n ``` is the number of elements in the current [` Vector `](/apis/Classes/HH/Vector/) prior to the call to
[` pop() `](/apis/Classes/HH/Vector/pop/).




## Returns




+ ` Tv ` - The value of the last element.




## Examples




This example shows that [` pop() `](/apis/Classes/HH/Vector/pop/) returns the last element and removes it from the [` Vector `](/apis/Classes/HH/Vector/):




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$last_color = $v->pop();

\var_dump($last_color);
\var_dump($v);
```




This example shows that trying to ` pop ` from an empty [` Vector `](/apis/Classes/HH/Vector/) will throw an exception:




``` throw-exception.hack
$v = Vector {};

$last_element = $v->pop(); // Throws InvalidOperationException
```
<!-- HHAPIDOC -->
