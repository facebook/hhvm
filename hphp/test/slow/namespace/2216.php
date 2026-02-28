<?hh

namespace my\name;
class MyClass {
}
function myfunction() :mixed{
}
const MYCONST = 123;


<<__EntryPoint>>
function main_2216() :mixed{
  $a = new MyClass;
  \var_dump(\get_class($a));
  $c = new \my\name\MyClass;
  \var_dump(\get_class($a));
  $a = \strlen('hi');
  \var_dump($a);
  $d = namespace\MYCONST;
  \var_dump($d);
  $d = __NAMESPACE__ . '\MYCONST';
  \var_dump(\constant($d));
  \var_dump(\defined('MYCONST'));
}
