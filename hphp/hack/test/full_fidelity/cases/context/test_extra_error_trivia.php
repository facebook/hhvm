<?hh

class C {

  // 'sta' becomes error trivia
  public sta function f() {
  }

  // 'meaninglessword' becomes error trivia
  public static meaninglessword function f() {
  }

  // no change in parsing
  public meaninglessword static function f() {
  }

  // no change in parsing
  public meaninglessword1 meaninglessword2 function f() {
  }

}
