<?php


function log_me() {
  $message = call_user_func_array('sprintf', func_get_args());
  return rtrim($message)."\n";
}

final class WTFer {
  private static $boo;
  public static function run() {
    $i = 0;
    while ($i < 100000) {
      self::blah($i++);
    }
  }

  private static function blah($i) {
    $t = 0;

    foreach (array(&self::$boo) as $k => &$v) {
      $v = 0;
    }

    self::$boo = $t;
    log_me('next');
  }
}

WTFer::run();
echo "ok\n";
