<?hh

// TODO(t11082787): identifying generics
class C<TFoo> {
  public function test(): TFoo {
    //UNSAFE
  }
}
