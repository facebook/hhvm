<?hh

class EE extends Exception {
}
class E extends EE {
  function foo() :mixed{
}
  function __construct() {
    echo 'MAKING E';
    parent::__construct();
  }
}

<<__EntryPoint>>
function main_656() :mixed{
new E;
}
