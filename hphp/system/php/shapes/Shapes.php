<?hh // partial

namespace HH {
  abstract final class Shapes {

    <<__Rx>>
    public static function idx(
      darray $shape,
      arraykey $index,
      $default = null,
    ) {
      return \hphp_array_idx($shape, $index, $default);
    }

    <<__Rx>>
    public static function keyExists(
      darray $shape,
      arraykey $index,
    ): bool {
      return \array_key_exists($index, $shape);
    }

    <<__Rx>>
    public static function removeKey(
      inout darray $shape,
      arraykey $index,
    ): void {
      unset($shape[$index]);
    }

    <<__Rx>>
    public static function toArray(
      darray $shape,
    ): darray {
      return $shape;
    }

    <<__Rx>>
    public static function toDict(
      darray $shape,
    ): dict {
      return dict($shape);
    }

  }
}
