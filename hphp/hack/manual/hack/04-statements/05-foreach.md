# Foreach

The `foreach` statement iterates over the set of elements in a given collection, starting at the beginning, executing a single statement
each iteration. On each iteration, the value of the current element is assigned to the corresponding variable, as specified. The loop body
is executed zero or more times. For example:

```hack
$colors = vec["red", "white", "blue"];
foreach ($colors as $color) {
  // ...
}
```

Here, we iterate over a collection of three strings in a vec of `string`. Inside the loop body, `$color` takes on the value of the current string.

As each array element has an index as well as a value, we can access both. For example:

```hack
$colors = vec["red", "white", "blue"];
foreach ($colors as $key => $color) {
  // ...
}
```

The `as` clause gives us access to the array key.

We can cause each element's value to be ignored, using `$_`, as follows:

```hack
  $a = dict['a' => 10, 'f' => 30];
  foreach ($a as $key => $_) { // 10 and 30 are ignored
    // ...
  }
```

We can also use `list()` here `foreach($vec_of_tuples as list($here, $there))` and here `foreach($vec_of_tuples as $key => list($here, $there))`.
For more information about lists, see [list](/hack/expressions-and-operators/list).
