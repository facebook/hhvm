<?hh

class C {
  public function f() {
                          // ERROR RECOVERY: report missing brace here
  public function g() {
  }
} // instead of reporting it here, as was formerly done.

// note that if there *is* a '}' at (9,1), the new behavior will report
// two distinct errors: "missing '}' on line 5," and "extra '}' on line 9."
