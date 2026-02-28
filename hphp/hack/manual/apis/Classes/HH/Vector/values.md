
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function values(): Vector<Tv>;
```




Essentially a copy of the current [` Vector `](/apis/Classes/HH/Vector/).




This method is interchangeable with [` toVector() `](/apis/Classes/HH/Vector/toVector/).




## Returns




+ [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example shows how [` values() `](/apis/Classes/HH/Vector/values/) is identical to [` toVector() `](/apis/Classes/HH/Vector/toVector/). It returns a deep copy of `` $v ``, so mutating this new [` Vector `](/apis/Classes/HH/Vector/) doesn't affect the original.




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Make a deep Vector copy of $v
$v2 = $v->values();

// Modify $v2 by adding an element
$v2->add('purple');
\var_dump($v2);

// The original Vector $v doesn't include 'purple'
\var_dump($v);
```
<!-- HHAPIDOC -->
