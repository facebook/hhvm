<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function meep($a, $b) {
  throw new Exception('blah');
}

class Interceptor {
  private static $recurse = false;
  public static function io_intercept($name, $obj_or_cls, inout $args,
                                      $ctx, inout $done) {
    if (self::$recurse) {
      $done = false;
      return;
    }
    self::$recurse = true;
    return meep(...$args);
  }
}

<<__EntryPoint>> function main(): void {
  fb_intercept('meep', 'Interceptor::io_intercept', true);
  try {
    meep(1, 2);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
