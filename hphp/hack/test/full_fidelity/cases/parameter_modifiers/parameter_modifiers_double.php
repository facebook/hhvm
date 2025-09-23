<?hh

function f1(readonly readonly int $x): void {}
function f2((function(optional optional int $x): void) $f): void {}
function f3(named named int $x): void {}
function f3(readonly readonly int $x): void {}
