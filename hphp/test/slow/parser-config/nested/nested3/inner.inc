<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class C3 {
  static function inner() {
    readonly 1;
    var_dump(__METHOD__);
  }
}
