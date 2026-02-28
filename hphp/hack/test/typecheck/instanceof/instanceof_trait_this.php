<?hh

trait Tr {

  public static function perhaps(mixed $x): ?this {
    if ($x is this) {
      hh_show($x);
      return $x;
    }
    return null;
  }
}
