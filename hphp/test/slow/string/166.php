<?hh

function f() :mixed{
 return 'x';
 }
function g() :mixed{
}
function test1($a) :mixed{
  $buf = '';
  foreach ($a as $s) {
     $buf .= (string)(f()) . (string)(g()) . 'h' . (string)(f()) . 'h' . (string)(g());
  }
  foreach ($a as $s) {
    $buf .= ($s . 'h' . $s);
  }
  return $buf;
}
function test2() :mixed{
  return (string)(f()) . (string)(g()) . (string)(f()) . (string)(g());
}
function test3() :mixed{
  return (string)(f()) . (string)(g()) . (string)(f()) . (string)(g()) . (string)(f()) . (string)(g()) . (string)(f()) . (string)(g()) . (string)(f());
}
function test4() :mixed{
  $s = f();
  $s .=    ('foo'.    'bar'.    f().    'foo'.    'baz'.    f().    'fuz'.    'boo'.    f().    'fiz'.     'faz');
  $s .= f();
  return $s;
}
function test5() :mixed{
  return (string)(g()).(string)(g()).(string)(g()).(string)(g());
}
function test6() :mixed{
  return (string)(g()).(string)(f()).(string)(g());
}

<<__EntryPoint>>
function main_166() :mixed{
var_dump(test1(vec[1]));
var_dump(test2());
var_dump(test3());
var_dump(test4());
var_dump(test5());
var_dump(test6());
}
