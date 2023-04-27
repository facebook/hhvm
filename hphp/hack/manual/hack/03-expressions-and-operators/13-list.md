`list()` is special syntax for unpacking tuples. It looks like a function, but it isn't one. It can be used in positions that you would assign into.

```Hack
$tuple = tuple('one', 'two', 'three');
list($one, $two, $three) = $tuple;
echo "1 -> {$one}, 2 -> {$two}, 3 -> {$three}\n";
```

The `list()` will consume the `tuple` on the right and assign the variables inside of itself in turn.
If the types of the tuple elements differ, the `list()` syntax will make sure that the type information is preserved.

```Hack
<<__EntryPoint>>
function main(): void {
  $tuple = tuple('string', 1, false);
  list($string, $int, $bool) = $tuple;
  takes_string($string);
  takes_int($int);
  takes_bool($bool);
  echo 'The typechecker understands the types of list().';
}

function takes_string(string $_): void {}
function takes_int(int $_): void {}
function takes_bool(bool $_): void {}
```

You can use the special `$_` variable to ignore certain elements of the `tuple`. You can use `$_` multiple times in one assignment and have the types be different. You **MUST** use `$_` if you want to ignore the elements at the end. You are not allowed to use a `list()` with fewer elements than the length of the `tuple`.

```Hack
$tuple = tuple('a', 'b', 'c', 1, 2, 3);
list($_, $b, $c, $_, $two, $_) = $tuple;
echo "b -> {$b}, c -> {$c}, two -> {$two}\n";
```

If the RHS and the LHS of a `list()` are referring to the same variables, the behavior is undefined. As of hhvm 4.46, the typechecker will **NOT** warn you when you make this mistake! HHVM will also not understand what you mean. Do **NOT** do this.

```Hack
$a = tuple(1, 2);
// BAD, since $a is used on the right and on the left!
list($a, $b) = $a;
```

You may also use `list()` on a `vec<T>`, but it is not recommended.

`list()` can be nested inside of another `list()` to unpack `tuples` from within `tuples`.

```Hack
$tuple = tuple('top level', tuple('inner', 'nested'));
list($top_level, list($inner, $nested)) = $tuple;
echo "top level -> {$top_level}, inner -> {$inner}, nested -> {$nested}\n";
```

My personal favorite place to put a `list()` is inside a `foreach($vec_of_tuples as list($one, $two, $three))`.

```Hack
$vec_of_tuples = vec[
  tuple('A', 'B', 'C'),
  tuple('a', 'b', 'c'),
  tuple('X', 'Y', 'Z'),
];

foreach ($vec_of_tuples as list($first, $second, $third)) {
  echo "{$first} {$second} {$third}\n";
}
```
