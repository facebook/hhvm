<?hh

function f() :mixed{

  Compilation1317::$g++;
}

<<__EntryPoint>>
function main_1317() :mixed{
var_dump((bool)f(),(int)f(),(float)f(),(string)f());
}

abstract final class Compilation1317 {
  public static $g = 0;
}
