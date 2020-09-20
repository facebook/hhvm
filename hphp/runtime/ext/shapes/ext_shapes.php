<?hh // strict

namespace HH {
  abstract final class Shapes {

    <<__Native, __Pure, __IsFoldable>>
    public static function idx(
      ?darray $shape,
      arraykey $index,
      mixed $default = null,
    ): mixed;

    <<__Pure>>
    public static function at(
      darray $shape,
      arraykey $index,
    ) {
      return $shape[$index];
    }

    <<__Pure>>
    public static function keyExists(
      darray $shape,
      arraykey $index,
    ): bool {
      return \array_key_exists($index, $shape);
    }

    <<__Pure>>
    public static function removeKey(
      inout darray $shape,
      arraykey $index,
    ): void {
      unset($shape[$index]);
    }

    <<__Pure>>
    public static function toArray(
      darray $shape,
    ): darray {
      return $shape;
    }

    <<__Pure>>
    public static function toDict(
      darray $shape,
    ): dict {
      return dict($shape);
    }

  }
}
