<?hh
use X\Y as test, X\Z as test2;

require "ns_065.inc";
<<__EntryPoint>> function main(): void {
test\foo();
test2\foo();
}
