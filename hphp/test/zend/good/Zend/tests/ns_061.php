<?hh
class A {}
use \A as B;
<<__EntryPoint>> function main(): void {
echo get_class(new B)."\n";
}
