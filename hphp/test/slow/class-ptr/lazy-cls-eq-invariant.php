<?hh

final class C {}

<<__EntryPoint>>
function main(): void {
  $a = C::class;
  $c = HH\get_class_from_object(new C());

  var_dump($a === $c);
  if ($a === $c) {
    var_dump("they are equal");
  }
  invariant(
    $a === $c,
    "Failed: %s <> %s",
    $a,
    $c,
  );
}
