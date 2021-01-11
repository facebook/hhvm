<?hh

function f()[_]: void {}

function hof((function ()[_]: void) $f): void {}
