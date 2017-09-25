////file1.php
<?hh
function getArray(): array {
//UNSAFE
}

function mapFromKeys<Tk, Tv>(
  Traversable<Tk> $keys,
  (function(Tk): Tv) $map_key_to_value_function,
): Map<Tk, Tv> {
  //UNSAFE
}

////file2.php
<?hh // strict

class C { }
  function foo(array<classname<C>> $a) : Map<string, classname<C>> {
    return mapFromKeys(getArray(), $t ==> $t);
}
