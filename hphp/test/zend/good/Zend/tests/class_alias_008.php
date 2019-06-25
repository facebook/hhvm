<?hh

abstract class foo { }

class_alias('foo', "\0");
<<__EntryPoint>> function main(): void {
$a = "\0";

new $a;
}
