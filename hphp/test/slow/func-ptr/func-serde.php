<?hh

function test() {
  return 1;
}

function serde($val) {
  print "* serialize result:\n";
  $ser = serialize($val);
  var_dump($ser);

  print "* unserialize result:\n";
  $unser = unserialize($ser);
  var_dump($unser);
}

function fb_serde($val) {
  print "* fb_serialize result:\n";
  $ser = fb_serialize($val);
  var_dump($ser);

  print "* fb_unserialize result:\n";
  $ret = false;
  $unser = fb_unserialize($ser, inout $ret);
  var_dump($ret, $unser);
}

function json_serde($val) {
  print "* json_encode result:\n";
  $encode = json_encode($val);
  var_dump($encode);

  print "* json_decode result:\n";
  $decode = json_decode($encode);
  var_dump($decode);
}

<<__EntryPoint>>
function main() {
  $func = fun('test');

  print_r($func);
  print "\n";

  var_export($func);
  print "\n";

  debug_zval_dump($func);
  var_dump($func);
  print "\n";

  serde($func);
  fb_serde($func);
  json_serde($func);
}
