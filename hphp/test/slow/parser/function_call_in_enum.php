<?hh

function f() { return 1; }
enum E : string { X = f(); }

