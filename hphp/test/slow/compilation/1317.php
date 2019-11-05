<?hh

function f() {

  Compilation1317::$g++;
}

<<__EntryPoint>>
function main_1317() {
var_dump((bool)f(),(int)f(),(float)f(),(string)f());
var_dump((array)f());
}

abstract final class Compilation1317 {
  public static $g;
}
