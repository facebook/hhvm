<?hh

function f1((function(optional readonly bool): void) $_): void {}
function f2((function(readonly optional bool): void) $_): void {}
