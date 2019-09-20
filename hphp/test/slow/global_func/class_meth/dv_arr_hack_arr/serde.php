<?hh

class A {
  static public function func1() {
    return 1;
  }
}

function serde($x) {
  print "* serialize result:\n";
  $ser = serialize($x);
  var_dump($ser);

  print "* unserialize result:\n";
  var_dump(unserialize($ser));
}

function fb_serde($x) {
  print "* fb_serialize result:\n";
  $ser = fb_serialize($x, FB_SERIALIZE_HACK_ARRAYS);
  var_dump($ser);

  print "* fb_unserialize result:\n";
  $ret = false;
  $unser = fb_unserialize($ser, inout $ret, FB_SERIALIZE_HACK_ARRAYS);
  if ($ret) {
    var_dump($unser);
  }
}

function json_serde($x) {
  print "* json_encode result:\n";
  $encode = json_encode($x);
  var_dump($encode);

  print "* json_decode result:\n";
  var_dump(json_decode($encode));
}

<<__EntryPoint>>
function main() {
  $x = HH\class_meth(A::class, 'func1');
  print_r($x);
  print "\n";

  var_export($x);
  print "\n";

  var_dump($x);
  debug_zval_dump($x);

  serde($x);
  fb_serde($x);
  json_serde($x);
}
