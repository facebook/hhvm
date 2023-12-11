<?hh

class X        { public static $x = dict[]; }
function go()  :mixed{ X::$x[0] = 2; }
function go2() :mixed{ var_dump((bool)X::$x); }


<<__EntryPoint>>
function main_public_static_props_013() :mixed{
go();
go2();
}
