<?hh

class foo {
  function __construct($arrayobj) {
    var_dump($arrayobj);
  }
}

<<__EntryPoint>>
function main() {
  new foo(array(new stdClass));
}
