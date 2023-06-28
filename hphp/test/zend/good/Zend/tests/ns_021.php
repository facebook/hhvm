<?hh
namespace test;

class Test {
    static function foo() :mixed{
        echo __CLASS__,"::",__FUNCTION__,"\n";
    }
}

function foo() :mixed{
    echo __FUNCTION__,"\n";
}
<<__EntryPoint>> function main(): void {
foo();
\test\foo();
\test\Test::foo();
}
