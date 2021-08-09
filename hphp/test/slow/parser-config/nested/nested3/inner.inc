<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class C3 {
  readonly static function inner() {
    var_dump(__METHOD__);
  }
}
