<?hh

function equal_by<Tk as arraykey, Tv>(
  dict<Tk, Tv> $dict1,
  dict<Tk, Tv> $dict2,
  (function(Tv, Tv)[_]: bool) $comparator,
)[ctx $comparator]: bool {
  return false;
}

// flib/ads/creation_package/dynamic_calculator/AdCreationPackageDynamicDefaultCalculator.php:43
function repro(
  dict<arraykey, mixed> $values,
  dict<int, vec<string>> $input_fields,
): void {
  $values_fields = HH\Lib\Dict\map(
    $values,
    $sub_value ==> {
      if ($sub_value is dict<_, _>) {
        $result = $sub_value;
      } else {
        $result = dict[];
      }
      $keys = HH\Lib\Vec\keys($result);
      return $keys;
    },
  );
  HH\Lib\Vec\map($values_fields, $f ==> {
    keyset($f);
    return null;
  });
}
