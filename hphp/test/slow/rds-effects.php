<?hh

abstract final class C {
  public static darray<string, mixed> $cache = dict[];
}

<<__NEVER_INLINE>>
function getchar($name) :mixed{
  $cached = idx(C::$cache, $name);
  if ($cached !== null) return $cached;
  // This line will raise a notice. In the error handler, we may update
  // C:$cache, so we can't assume that its value remains unchanged here.
  $new = $name[2];
  C::$cache[$name] = $new;
  return $new;
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler((...$args) ==> {
    for ($i = 0; $i < 17; $i++) {
      C::$cache[$i] = $i;
    }
  });

  print(getchar('test')."\n");
  print(getchar('no')."\n");
}
