<?hh

abstract final class DynamicFunctions1184 {
  public static $a;
}
<<__DynamicallyCallable>>
function test() :mixed{
 return DynamicFunctions1184::$a;
}
<<__EntryPoint>> function main(): void {
DynamicFunctions1184::$a = 'test';
$b = (DynamicFunctions1184::$a)();
$b = 'ok';
var_dump(DynamicFunctions1184::$a);
}
