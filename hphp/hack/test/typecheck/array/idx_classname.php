<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class C {
  private static function getClassVec(): vec<classname<this>> {
    return vec[];
  }

  private static function uhh(classname<this> $class): void {
    $classvec = self::getClassVec();
    idx($classvec, $class); // Produce warning for this
    $classvec[$class]; // Invalid index type for this vec (4449)
  }
}
