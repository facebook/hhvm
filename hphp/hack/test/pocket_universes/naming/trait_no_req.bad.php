<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

trait myTrait0 {
    enum E {
        :@S(type T = string, val = "foo");
    }
}
