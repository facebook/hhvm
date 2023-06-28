<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function meep($a, $b) :mixed{
  throw new Exception('blah');
}

class Interceptor {
  private static $recurse = false;
  public static function io_intercept($name, $obj_or_cls, inout $args) :mixed{
    if (self::$recurse) {
      return shape();
    }
    self::$recurse = true;
    return shape('callback' => 'meep');
  }
}

<<__EntryPoint>> function main(): void {
  fb_intercept2('meep', 'Interceptor::io_intercept');
  try {
    meep(1, 2);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
