<?hh

class C {
  public function f() {
    if ($x // ERROR RECOVERY: report missing right paren and right brace here

  public function g() {
  }
}

class C {
  public function f() {
    if ($x // ERROR RECOVERY: report missing right paren and right brace here

  private function g() {
  }
}

class C {
  public function f() {
    if ($x // ERROR RECOVERY: report missing right paren and right brace here

  protected function g() {
  }
}

// there are three of basically the same case just to make sure that
// 'public', 'private', and 'protected' are all treated the same in
// with respect to error recovery.
