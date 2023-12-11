<?hh

class foo {
  function __construct($arrayobj) {
    var_dump($arrayobj);
  }
}

<<__EntryPoint>>
function main() :mixed{
  new foo(vec[new stdClass]);
}
