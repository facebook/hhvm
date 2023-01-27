Any function containing the `yield` operator is a *generator function*. A generator function generates a collection of zero or more
key/value pairs where each pair represents the next element in some series. For example, a generator might yield random numbers or
the series of Fibonacci numbers. When a generator function is called explicitly, it returns an object of type `Generator`, which
implements the interface `Iterator`. As such, that object can be iterated over using the `foreach` statement. During each iteration,
the runtime calls the generator function implicitly to get the element. Then the runtime saves the state of the generator for subsequent
element-fetch requests. Consider the following example:

```Hack
function series(
  int $start,
  int $end,
  int $incr = 1,
): \Generator<int, int, void> {
  for ($i = $start; $i <= $end; $i += $incr) {
    yield $i;
  }
}

<<__EntryPoint>>
function main(): void {
  foreach (series(5, 15, 2) as $key => $val) {
    echo "key: $key, value: $val\n";
  }
  echo "-----------------\n";

  foreach (series(25, 20, 3) as $key => $val) {
    echo "key: $key, value: $val\n";
  }
  echo "-----------------\n";
}
```

Function `series` returns an instance of the generic type `Generator`, in which the first two type arguments are the element's key- and
value-type, respectively. (We won't discuss the third type argument here; it is `void` in all the examples below.)  Note that the function
does **not** contain any `return` statement. Instead, elements are *returned* as directed by the `yield` expression. As shown, the function
yields element values, one per loop iteration.

In `main`, we call `series` to generate the collection, and then we iterate over that collection from 5-15 in steps of 2, and simply display
the next element's key and value. However, when we use the range 25-20 in steps of 3, the resulting collection is empty, as 25 is already greater than 20.

In its simplest form, `yield` is followed by the value of the next element with that value's key being an `int` whose value starts at zero
for each collection. This is demonstrated in the output, which has keys 0-5.

`yield` can also specify the value of a key; for example:

```Hack
function squares(
  int $start,
  int $end,
  string $keyPrefix = "",
): Generator<string, int, void> {
  for ($i = $start; $i <= $end; ++$i) {
    yield $keyPrefix.$i => $i * $i; // specify a key/value pair
  }
}

<<__EntryPoint>>
function main(): void {
  foreach (squares(-2, 3, "X") as $key => $val) {
    echo "key: $key, value: $val\n";
  }
}
```

By using the same syntax as that to initialize a dict element, we can provide both the key and associated value. Of course, the return
type of `squares` now uses `string` as the first generic type argument, as the element type has a key of type `string`.

The following example uses `yield` to generate a collection of strings, each of which is a record from a text file:

```Hack
function getTextFileLines(string $filename): Generator<int, string, void> {
  $infile = fopen($filename, 'r');
  if ($infile === false) {
    // handle file-open failure
  }

  try {
    while (true) {
      $textLine = fgets($infile);
      if ($textLine === false) {
        break;
      }
      $textLine = rtrim($textLine, "\r\n"); // strip off line terminator
      yield $textLine;
    }
  } finally {
    fclose($infile);
  }
}
```
