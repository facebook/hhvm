<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class C2 {
  readonly static function inner() {
    var_dump(__METHOD__);
  }
}
