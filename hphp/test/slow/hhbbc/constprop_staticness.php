<?hh

function a() :mixed{ return 'a'; }
function b() :mixed{ return 'b'; }

function foo() :mixed{
  $a = a();
  $b = b();
  return $a . $b;
}

function heh() :mixed{
  var_dump(foo());
}


<<__EntryPoint>>
function main_constprop_staticness() :mixed{
heh();
}
