<?hh

class A {}

function serde($val) :mixed{
  print "* serialize result:\n";
  $ser = serialize($val);
  var_dump($ser);

  print "* unserialize result:\n";
  $unser = unserialize($ser);
  var_dump($unser);
}

function fb_serde($val) :mixed{
  print "* fb_serialize result:\n";
  $ser = fb_serialize($val);
  var_dump($ser);

  print "* fb_unserialize result:\n";
  $ret = false;
  $unser = fb_unserialize($ser, inout $ret);
  var_dump($ret, $unser);
}

function json_serde($val) :mixed{
  print "* json_encode result:\n";
  $encode = json_encode($val);
  var_dump($encode);

  print "* json_decode result:\n";
  $decode = json_decode($encode);
  var_dump($decode);
}

<<__EntryPoint>>
function main() :mixed{
  $cls = __hhvm_intrinsics\create_class_pointer('A');

  print_r($cls);
  print "\n";

  var_export($cls);
  print "\n";

  debug_zval_dump($cls);
  var_dump($cls);
  print "\n";

  serde($cls);
  fb_serde($cls);
  json_serde($cls);
}
