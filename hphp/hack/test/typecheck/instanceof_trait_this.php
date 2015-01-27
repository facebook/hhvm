<?hh

trait Tr {

  public static function perhaps($x): ?this {
    if ($x instanceof static) {
      hh_show($x);
      return $x;
    }
    return null;
  }
}
