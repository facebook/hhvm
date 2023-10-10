<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function pub_stat() :mixed{}
  <<__DynamicallyCallable>>
  public static function pub_stat_dyn() :mixed{}

  public function pub() :mixed{}
  <<__DynamicallyCallable>>
  public function pub_dyn() :mixed{}

  private static function priv_stat() :mixed{}

}

<<__EntryPoint>>
function main() :mixed{
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
