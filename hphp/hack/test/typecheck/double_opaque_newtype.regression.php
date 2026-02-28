////n.php
<?hh

newtype N<+Thint> as nonnull = M<Thint>;

final class Utils {
  public static function to<<<__Explicit>> T>(mixed $_): N<T> {
    return mk_m<T>();
  }
}

////m.php
<?hh

newtype M<+Thint> as int = int;

function mk_m<<<__Explicit>> T>(): M<T> {
  return 0;
}

////usage.php
<?hh

function usage(mixed $result): int {
  return Utils::to<mixed>($result);
}
