<?hh // decl

class x {
  function f(inout $a) { var_dump(__METHOD__); }
  function g($f) { self::$f(inout $f); }
  function h($f) { $this->$f(inout $f); }
}

class y extends x {
  function f(inout $a) { var_dump(__METHOD__); }
}

<<__EntryPoint>>
function main() {
  (new y)->g('f');
  Y::g('f');
  (new y)->h('f');
}
