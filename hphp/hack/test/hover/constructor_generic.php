<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class Thingy<T> {
  /**
   * Make a Thingy
   */
  public function __construct(public T $item) {}
}

function testit():void {
  $x = new Thingy(3);
  //       ^ hover-at-caret
}
