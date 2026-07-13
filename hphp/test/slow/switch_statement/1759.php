<?hh

function id($x) :mixed{
 return $x;
 }
function ret_false($x) :mixed{
 return false;
 }
function assign(inout $x, $v) :mixed{
 $x = $v;
 return $v;
 }
function f($x) :mixed{
  switch ($x) {
  case ret_false(assign(inout $x, 32));
 echo 'fail';
 break;
  case id(assign(inout $x, 5)): echo 'here';
 break;
  default: echo 'default';
  }
}

<<__EntryPoint>>
function main_1759() :mixed{
f(32);
}
