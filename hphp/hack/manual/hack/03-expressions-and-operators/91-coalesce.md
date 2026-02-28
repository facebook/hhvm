# Coalesce

Given the expression `e1 ?? e2`, if `e1` is defined and not `null`, then the
result is `e1`. Otherwise, `e2` is evaluated, and its value becomes the result.
There is a sequence point after the evaluation of `e1`.

```hack
$nully = null;
$nonnull = 'a string';
\print_r(vec[
  $nully ?? 10,    // 10 as $nully is `null`
  $nonnull ?? 10,  // 'a string' as $nonnull is `nonnull`
]);

$arr = dict['black' => 10, 'white' => null];
\print_r(vec[
  $arr['black'] ?? -100,  // 10 as $arr['black'] is defined and not null
  $arr['white'] ?? -200,  // -200 as $arr['white'] is null
  $arr['green'] ?? -300,  // -300 as $arr['green'] is not defined
]);
```

It is important to note that the right-hand side of the `??` operator will be
conditionally evaluated. If the left-hand side is defined and not `null`, the
right-hand side will not be evaluated.

```hack no-extract
$nonnull = 4;

// The `1 / 0` will never be evaluated and no Exception is thrown.
$nonnull ?? 1 / 0;

// The function_with_sideeffect is never invoked.
$nonnull ?? function_with_sideeffect();
```


## `??` and `idx()`

The `??` operator is similar to the built-in function `idx()`. However, an
important difference is that `idx()` only falls back to the specified default
value if the given key does not exist, while `??` uses the fallback value even
if a key exists but has `null` value. Compare these examples to the ones above:

```hack
$arr = dict['black' => 10, 'white' => null];
\print_r(vec[
  idx($arr, 'black', -100),  // 10
  idx($arr, 'white', -200),  // null
  idx($arr, 'green', -300),  // -300
  idx($arr, 'green'),        // null
]);
```


## Coalescing assignment operator

A coalescing
[assignment](/hack/expressions-and-operators/assignment)
operator `??=` is also available.

The `??=` operator can be used for conditionally writing to a variable if it is
null, or to a collection if the specified key is not present or has `null`
value.

This is similar to `e1 = e1 ?? e2`, with the important difference that `e1` is
only evaluated once.

The `??=` operator is very useful for initializing elements of a collection:

```hack
function get_counts_by_value(Traversable<string> $values): dict<string, int> {
  $counts_by_value = dict[];
  foreach ($values as $value) {
    $counts_by_value[$value] ??= 0;
    ++$counts_by_value[$value];
  }
  return $counts_by_value;
}

function get_people_by_age(
  KeyedTraversable<string, int> $ages_by_name,
): dict<int, vec<string>> {
  $people_by_age = dict[];
  foreach ($ages_by_name as $name => $age) {
    $people_by_age[$age] ??= vec[];
    $people_by_age[$age][] = $name;
  }
  return $people_by_age;
}

<<__EntryPoint>>
function main(): void {
  $values = vec['foo', 'bar', 'foo', 'baz', 'bar', 'foo'];
  \print_r(get_counts_by_value($values));

  $people = dict[
    'Arthur' => 35,
    'Ford' => 110,
    'Trillian' => 35,
    'Zaphod' => 120,
  ];
  \print_r(
    get_people_by_age($people)
    |> Dict\map($$, $names ==> Str\join($names, ', '))
  );
}
```
