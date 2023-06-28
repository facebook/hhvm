<?hh

function f() :mixed{ return 1; }
enum E : string { X = f(); }

