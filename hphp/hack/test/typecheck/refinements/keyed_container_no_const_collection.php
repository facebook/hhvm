<?hh

function my_array_keys<Tk as arraykey>(
  KeyedContainer<Tk, mixed> $input,
)[]: vec<Tk> {
  return vec[];
}

final class MyClass {
  public static function maybeClsMethToVarray(mixed $value)[]: vec<string> {
    return vec[];
  }

  public static function mymethod(mixed $a, mixed $b, bool $test): void {
    if ($a is KeyedContainer<_, _> && !($a is ConstCollection<_>)) {
      $v = self::maybeClsMethToVarray($a);
      $a = $test ? $v : $a;
      $a_keys = my_array_keys($a);
      foreach ($a_keys as $idx => $_) {
        $key_a_result = $a_keys[$idx];
        $current_a_result = $a[$key_a_result];
      }
    }
  }
}
