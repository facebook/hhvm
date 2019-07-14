<?hh
namespace Exception;
function foo() { echo "ok\n"; }

<<__EntryPoint>> function main(): void {
\Exception\foo();
\Exception::bar();
}
