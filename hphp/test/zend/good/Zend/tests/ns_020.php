<?hh
namespace X;
use X as Y;
function foo() {
    echo __FUNCTION__,"\n";
}
<<__EntryPoint>> function main(): void {
foo();
\X\foo();
Y\foo();
\X\foo();
}
