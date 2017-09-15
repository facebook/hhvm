<?hh

class C { public $heh; }

function test_array() {
  $arr = array('lol' => 'sup');
  $in = serialize($arr);
  return unserialize($in);
}

function test_dyn_prop() {
  $c = new C();
  $c->lol = 'sup';
  $in = serialize($c);
  return unserialize($in);
}

function test_prop_order() {
  $in = 'O:1:"C":2:{s:3:"sup";N;s:3:"heh";s:3:"sup";}';
  return unserialize($in);
}

function main() {
  test_array();
  test_dyn_prop();
  test_prop_order();
}

main();
