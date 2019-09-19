<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function pub_stat() {}
  <<__DynamicallyCallable>>
  public static function pub_stat_dyn() {}

  public function pub() {}
  <<__DynamicallyCallable>>
  public function pub_dyn() {}

  private static function priv_stat() {}

}

<<__EntryPoint>>
function main() {
  $a = 'A';
  $pub_stat = 'pub_stat';
  $pub_stat_dyn = 'pub_stat_dyn';
  $a_pub_stat = 'A::pub_stat';
  $a_pub_stat_dyn = 'A::pub_stat_dyn';

  echo "============== Don't log: new ================\n";
  $a::pub_stat();

  echo "============== Don't log =================\n";
  $a::pub_stat_dyn();
  $a_pub_stat_dyn();

  echo "============== Log ======================\n";
  $a::$pub_stat();
  A::$pub_stat();
  $a_pub_stat();
}
