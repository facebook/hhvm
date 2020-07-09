<?hh
use X\Y as test, X\Z as test2;

<<__EntryPoint>> function main(): void {
require "ns_065.inc";
test\foo();
test2\foo();
}
