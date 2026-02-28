<?hh
namespace Exception;
function foo() :mixed{ echo "ok\n"; }

<<__EntryPoint>> function main(): void {
\Exception\foo();
\Exception::bar();
}
