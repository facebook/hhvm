<?hh

class Klass {
  // ensure we gracefully handle an empty block, which can have weird positions (Pos.none)
  private function foo(): void {}
  public function bar(): void {
    $this->/*range-start*/foo/*range-end*/();
  }
}
