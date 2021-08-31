<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class C2 {
  static function inner() {
    readonly 1;
    var_dump(__METHOD__);
  }
}
