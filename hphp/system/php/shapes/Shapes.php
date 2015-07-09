<?hh

namespace HH {
  abstract final class Shapes {

    public static function idx(
      array $shape,
      arraykey $index,
      $default = null,
    ) {
      return \hphp_array_idx($shape, $index, $default);
    }

    public static function keyExists(
      array $shape,
      arraykey $index,
    ): bool {
      return \array_key_exists($index, $shape);
    }

    public static function removeKey(
      array &$shape,
      arraykey $index,
    ): void {
      unset($shape[$index]);
    }

    public static function toArray(
      array $shape,
    ): array {
      return $shape;
    }
  }
}
