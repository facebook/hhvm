<?hh
namespace test\ns1;

class Exception {
}
<<__EntryPoint>> function main(): void {
echo \get_class(new Exception()),"\n";
}
