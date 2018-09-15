<?hh
// Copyright 2016 Facebook. All Rights Reserved.

abstract class FooBar {
  final public static function blah( ): this {
    try {
      while (true) {
        self::do_something();
      }
    } catch (Exception $e) {
    }
  }
}
