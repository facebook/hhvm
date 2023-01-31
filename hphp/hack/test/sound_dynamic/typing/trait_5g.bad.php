<?hh

// When typechecking T, whether the this type should be considered SDT
// or not depends ultimately by the class that uses the trait.  As a
// safe approximation, we do not consider T itself to be SDT, but we
// propagate the SDT attribute to all the methods of T.

// In presence of require extends constraints, we can take into
// account if the trait constraint ensures SDTness.  The code below is
// however rejected because the trait T itself is not declared SDT.

trait T {
  require extends C;
  public function f(): dynamic {
    return $this;
  }
}

<<__SupportDynamicType>>
class C {
}
