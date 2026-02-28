
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Splice the current [` Vector `](/apis/Classes/HH/Vector/) in place




``` Hack
public function splice(
  int $offset,
  ?int $len = NULL,
): void;
```




This function provides the functionality of
[` array_splice() `](<http://php.net/manual/en/function.array-splice.php>)
for [` Vector `](/apis/Classes/HH/Vector/)s (except that [` splice() `](/apis/Classes/HH/Vector/splice/) does not permit specifying
replacement values.  If a third ("replacement values") parameter is
specified, an exception is thrown.










Note that this function modifies the current [` Vector `](/apis/Classes/HH/Vector/) in place.




## Parameters




+ ` int $offset ` - The (0-based) key at which to begin the splice. If
  negative, then it starts that far from the end of the
  current [` Vector `](/apis/Classes/HH/Vector/).
+ ` ?int $len = NULL ` - The length of the splice. If `` null ``, then the current
  [` Vector `](/apis/Classes/HH/Vector/) is spliced until its end.




## Returns




* ` void `




## Examples




The following example shows how to use ` $offset ` and `` $len `` together:




``` basic-usage.hack
// Remove the element at index 2:
$v = Vector {'red', 'green', 'blue', 'yellow'};
$v->splice(2, 1);
\var_dump($v); // $v contains 'red', 'green', 'yellow'

// Remove elements starting at index 2:
$v = Vector {'red', 'green', 'blue', 'yellow'};
$v->splice(2);
\var_dump($v); // $v contains 'red', 'green'

// Remove three elements starting at index 0:
$v = Vector {'red', 'green', 'blue', 'yellow'};
$v->splice(0, 3);
\var_dump($v); // $v contains 'yellow'

// Remove elements starting two positions from the end:
$v = Vector {'red', 'green', 'blue', 'yellow'};
$v->splice(-2);
\var_dump($v); // $v contains 'red', 'green

// Remove elements starting at index 0 and stopping one position from the end:
$v = Vector {'red', 'green', 'blue', 'yellow'};
$v->splice(0, -1);
\var_dump($v); // $v contains 'yellow'
```
<!-- HHAPIDOC -->
