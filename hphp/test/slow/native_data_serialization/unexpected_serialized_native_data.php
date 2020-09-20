<?hh

class Foo {}
<<__EntryPoint>> function main(): void {
$foo = unserialize("O:3:\"Foo\":1:{s:7:\"\0native\";i:1337;}");
var_dump($foo);
}
