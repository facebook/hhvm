<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function mycount(readonly Container<mixed> $container)[]: int {
  return 0;
}

function myGetType<T>(string $key, TypeStructure<T> $type): T {
  throw new Exception("A");
}

abstract final class TypeBuilder {
  public static function string()[]: TypeStructure<string> {
    throw new Exception("A");
  }
  public static function vec<TVal>(
    TypeStructure<TVal> $v,
  )[]: TypeStructure<vec<TVal>> {
    throw new Exception("A");
  }
  public static function dict<TKey as arraykey, TVal>(
    TypeStructure<TKey> $k,
    TypeStructure<TVal> $v,
  )[]: TypeStructure<dict<TKey, TVal>> {
    throw new Exception("A");
  }
  public static function mixed()[]: TypeStructure<mixed> {
    throw new Exception("A");
  }
}

function test1():void {
  // Without this type annotation we get an error downstream
  // due to issues with array access checking under pessimisation
  // T180835046
  $sounds = myGetType<vec<dict<string,mixed>>>(
      "sounds",
      TypeBuilder::vec(
        TypeBuilder::dict(TypeBuilder::string(), TypeBuilder::mixed()),
      ),
    );
  foreach ($sounds as $s) {
    $position = $s["position"] as vec<_>;
    $x = mycount($position);
  }
}
