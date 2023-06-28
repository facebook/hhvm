<?hh

function y() :mixed{ return null; }
function x() :mixed{ return array_values(y()); }

<<__EntryPoint>>
function main_builtin_003() :mixed{
x();
}
