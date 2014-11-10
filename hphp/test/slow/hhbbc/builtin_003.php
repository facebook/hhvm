<?hh

function y() { return null; }
function x() { return array_values(y()); }
x();
