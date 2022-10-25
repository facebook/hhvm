<?hh


module a;

function bar1() { return foo<>; }
function bar2() { return "foo"; }
function bar3() { return () ==> { return foo(); }; }
function bar4() { return C::foo_static<>; }
function bar5() { return "C::foo_static"; }
function bar6() { return () ==> { return C::foo_static(); }; }
