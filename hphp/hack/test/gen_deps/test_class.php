<?hh // strict

class B {

}
class C {

}
class A {
  public static function ast() : B {
    // UNSAFE
  }


}

function hint_on_shape(): shape('a' => B)  {
  // UNSAFE
}

function optional_hint(): ?B {
  // UNSAFE
}

function function_hint(): (function(B,A):C) {
  // UNSAFE
}
