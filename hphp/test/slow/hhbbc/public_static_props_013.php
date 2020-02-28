<?hh

class X        { static $x = varray[]; }
function go()  { X::$x[0] = 2; }
function go2() { var_dump((bool)X::$x); }


<<__EntryPoint>>
function main_public_static_props_013() {
go();
go2();
}
