////file1.php
<?hh // partial
/* HH_FIXME[4110] */
function getArray(): array {
}

/* HH_FIXME[4110] */
function mapFromKeys<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  (function(Tk): Tv) $map_key_to_value_function,
): Map<Tk, Tv> {
}

////file2.php
<?hh // strict

class C { }
  function foo(array<classname<C>> $a) : Map<string, classname<C>> {
    return mapFromKeys(getArray(), $t ==> $t);
}
