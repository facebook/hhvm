<?hh
class A extends Exception { }
<<__EntryPoint>> function main(): void {
((new A));
print "parsed";
}
