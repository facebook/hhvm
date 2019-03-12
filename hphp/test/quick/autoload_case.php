<?hh
function __autoload($name) {
  var_dump($name);
  include 'autoload_case1.inc';
}
TestA::$D = 1;
