<?hh

function my_maybe_dict<Tk as arraykey, Tv>(
  ?KeyedTraversable<Tk, Tv> $dict_or_null,
): ?dict<Tk, Tv> {
  return $dict_or_null is null ? null : dict($dict_or_null);
}

final class MyFoo {
  private function getRawData(): dict<nothing, nothing> {
    return dict[];
  }

  final public function __getDirectDebitValuesUNSAFE(bool $test): ?shape(
    ?'bool_field' => bool,
  ) {
    $value = $this->getRawData();
    if ($value is nonnull) {
      if ($test) {
        $value = shape('bool_field' => true);
      } else {
        $value = shape();
      }
    }
    if ($value is nonnull) {
      $res = shape();
      if (Shapes::keyExists($value, 'bool_field')) {
        $inner_19 = Shapes::idx($value, 'bool_field');
        $res['bool_field'] = $inner_19 ?? false;
      }
      $value = $res;
    }
    return $value;
  }
}
