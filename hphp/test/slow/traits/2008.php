<?hh

interface a {
 }

abstract class b {
 }

final class c {
 }

trait d {
}
<<__EntryPoint>> function main(): void {
var_dump(class_exists('a'));
var_dump(class_exists('b'));
var_dump(class_exists('c'));
var_dump(class_exists('d'));
}
