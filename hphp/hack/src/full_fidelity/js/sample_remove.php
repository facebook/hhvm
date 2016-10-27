<?hh
// Copyright 2016 Facebook. All Rights Reserved.

class FooBar {
  protected ?string  $someString = null;
  /**
   * block comment
   */
  public function first() : this {
    while(self::whatever())
    { // Some comment
      /* leading comment */ try {
        self::do_something();
      } catch (Exception $e) {
      }
      self::do_something_else();
    }
    return $this;
  }

  public function second() : this {
    try {
      self::do_something();
    } catch (Exception $e) {
    } // trailing comment
    self::do_something_else();
    return $this;
  }
}
