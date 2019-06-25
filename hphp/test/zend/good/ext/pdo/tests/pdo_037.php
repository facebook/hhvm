<?hh

class MyStatement extends PDOStatement
{
}
<<__EntryPoint>> function main(): void {
$obj = new MyStatement;
var_dump($obj->foo());
}
