A type `?Foo` is either a value of type `Foo`, or `null`.

``` Hack
function takes_nullable_str(?string $s): string {
  if ($s is null){
    return "default";
  } else {
    return $s;
  }
}
```

`nonnull` is any value except `null`. You can use it to check if a
value is not `null`:

``` Hack
function takes_nullable_str2(?string $s): string {
  if ($s is nonnull){
    return $s;
  } else {
    return "default";
  }
}
```

This is slightly better than writing `$s is string`, as you don't need
to repeat the type name.

`nonnull` is also useful when using generics.

``` Hack
function my_filter_nulls<T as nonnull>(vec<?T> $items): vec<T> {
  $result = vec[];
  foreach ($items as $item) {
    if ($item is null) {
      // Discard it.
    } else {
      $result[] = $item;
    }
  }
  return $result;
}
```
