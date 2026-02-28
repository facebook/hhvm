<?hh

function id($x) :mixed{
 return $x;
 }
function ret_false($x) :mixed{
 return false;
 }
function f($x) :mixed{
  switch ($x) {
  case ret_false($x = 32);
 echo 'fail';
 break;
  case id($x = 5): echo 'here';
 break;
  default: echo 'default';
  }
}

<<__EntryPoint>>
function main_1759() :mixed{
f(32);
}
