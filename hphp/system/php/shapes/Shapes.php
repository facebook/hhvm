<?hh

namespace HH {
  abstract final class Shapes {

    public static function idx(
      darray $shape,
      arraykey $index,
      $default = null,
    ) {
      return \hphp_array_idx($shape, $index, $default);
    }

    public static function keyExists(
      darray $shape,
      arraykey $index,
    ): bool {
      return \array_key_exists($index, $shape);
    }

    public static function removeKey(
      darray &$shape,
      arraykey $index,
    ): void {
      unset($shape[$index]);
    }

    public static function toArray(
      darray $shape,
    ): darray {
      return $shape;
    }

    public static function toDict(
      darray $shape,
    ): dict {
      return dict($shape);
    }

  }
}
