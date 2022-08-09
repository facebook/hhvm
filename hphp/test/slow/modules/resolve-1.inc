<?hh

<<file:__EnableUnstableFeatures("modules")>>
module b;

function bar1() { return foo<>; }
function bar2() { return "foo"; }
function bar3() { return () ==> { return foo(); }; }
function bar4() { return C::foo_static<>; }
function bar5() { return "C::foo_static"; }
function bar6() { return () ==> { return C::foo_static(); }; }

function bar7() { return CInternal::foo_static<>; }
function bar8() { return "CInternal::foo_static"; }

function cbar1() { return CInternal::class; }
function cbar2() { return "CInternal"; }
function cbar3() { return () ==> { return CInternal::class; }; }
