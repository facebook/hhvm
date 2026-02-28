<?hh

function f1((function(optional inout bool): void) $_): void {}
function f2((function(inout optional bool): void) $_): void {}
