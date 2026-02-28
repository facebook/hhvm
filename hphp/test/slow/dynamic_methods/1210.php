<?hh

class z {
  function __construct() {
 echo 'construct';
 }
  function z() :mixed{
 echo 'method';
 }
}

<<__EntryPoint>>
function main_1210() :mixed{
$z = new z;
$z->z();
}
