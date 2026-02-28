<?hh

class C { public $heh; }

function test_array() :mixed{
  $arr = darray(dict['lol' => 'sup']);
  $in = serialize($arr);
  return unserialize($in);
}

function test_dyn_prop() :mixed{
  $c = new C();
  $c->lol = 'sup';
  $in = serialize($c);
  return unserialize($in);
}

function test_prop_order() :mixed{
  $in = 'O:1:"C":2:{s:3:"sup";N;s:3:"heh";s:3:"sup";}';
  return unserialize($in);
}

function main() :mixed{
  test_array();
  test_dyn_prop();
  test_prop_order();
}


<<__EntryPoint>>
function main_serialize() :mixed{
main();
}
