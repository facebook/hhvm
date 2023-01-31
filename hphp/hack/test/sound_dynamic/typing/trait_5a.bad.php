<?hh

// When typechecking T, whether the this type should be considered SDT
// or not depends ultimately by the class that uses the trait.  As a
// safe approximation, we do not consider T itself to be SDT, but we
// propagate the SDT attribute to all the methods of T.  This example
// will thus fail because in T->f $this </:D dynamic.

<<__SupportDynamicType>>
trait T {
  public function f(): dynamic {
    return $this;
  }
}
