<?hh
<<file:__EnableUnstableFeatures('readonly')>>

namespace HH {
  abstract final class Shapes {

    <<__IsFoldable>>
    public static function idx(
      ?darray $shape,
      arraykey $index,
      mixed $default = null,
    )[] {
      return idx($shape, $index, $default);
    }

    public static function at(
      darray $shape,
      arraykey $index,
    )[] {
      return $shape[$index];
    }

    public static function keyExists(
      readonly darray $shape,
      arraykey $index,
    )[]: bool {
      return \array_key_exists($index, $shape);
    }

    public static function removeKey(
      inout darray $shape,
      arraykey $index,
    )[]: void {
      unset($shape[$index]);
    }

    public static function toArray(
      darray $shape,
    )[]: darray {
      return $shape;
    }

    public static function toDict(
      darray $shape,
    )[]: dict {
      return dict(\HH\array_unmark_legacy($shape));
    }

  }
}
