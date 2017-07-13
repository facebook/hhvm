<?hh

class C {

  // parsed as a method containing an extra, invalid word: error1056
  public sta function f() {
  }

  // parsed as a method containing an extra, invalid word: error1056
  public static meaninglessword function f() {
  }

  // the below case isn't fixed yet; it retains its old behavior.
  public meaninglessword static function f() {
  }

  // the below case isn't fixed yet; it retains its old behavior.
  public meaninglessword1 meaninglessword2 function f() {
  }

}

class C {

  public sta // parsed as property declaration

  function f() { // parsed as method
  }

}

class C {

  public sta // parsed as property declaration

  public function f() { // parsed as method
  }

}
