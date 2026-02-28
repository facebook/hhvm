<?hh
function id($x,$y) :mixed{
 return $x;
 }
function id1($x) :mixed{
 return $x;
 }
function pid($x) :mixed{
 var_dump($x);
 return $x;
 }
class cls {
  function __construct() {
 print 'ctor
';
 }
  function f($x) :mixed{
 return $this;
 }
  function ttest() :mixed{
    return $this->f(pid('arg1'),pid('arg2'));
  }
}


<<__EntryPoint>>
function main_1507() :mixed{
error_reporting(E_ALL & ~E_NOTICE);
$d = id1(new cls())  ->f('arg1')  ->f('arg2')  ->f('arg3');
$d = id1(new cls())  ->f('arg1', 'argex1')  ->f('arg2', 'argex2')  ->f('arg3', 'argex3');
$d = id(new cls(), pid('idarg'))  ->f(pid('arg1'), pid('argex1'))  ->f(pid('arg2'), pid('argex2'))  ->f(pid('arg3'), pid('argex3'));
$d->ttest();
}
