<?hh
// Copyright 2016 Facebook. All Rights Reserved.
/* A two char variable in a comment: $x */
// Another: $x
abstract class FooBar {
  const string IN_A_STRING = '$x'; // Not a variable.
  protected string $a = ''; // A token
  public function abc(string $b) : this { // A token
    $yy = $b; // A token
    $zz = <foo>$x</foo>; // XHP body is not a variable usage.
    return $this;
  }
}
