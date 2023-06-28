<?hh

function test() :mixed{
  return 1;
}

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

function W($f) :mixed{
  try {
    var_dump($f());
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}


<<__EntryPoint>>
function main() :mixed{
  $func = test<>;

  print_r($func);
  print "\n";

  var_export($func);
  print "\n";

  debug_zval_dump($func);
  var_dump($func);
  print "\n";

  W(() ==> serde($func));
  W(() ==> fb_serde($func));
  W(() ==> json_serde($func));
}
