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
var_dump(trait_exists('a'));
var_dump(trait_exists('b'));
var_dump(trait_exists('c'));
var_dump(trait_exists('d'));
}
