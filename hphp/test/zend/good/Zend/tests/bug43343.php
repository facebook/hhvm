<?hh
namespace Foo;
class Bar { }
<<__EntryPoint>> function main(): void {
$foo = 'bar'; var_dump(new namespace::$foo);
}
