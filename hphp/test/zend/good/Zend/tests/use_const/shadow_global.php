<?hh

namespace {
    function test1() {
        var_dump(bar);
    }
}

namespace {
    use const foo\bar;
    
    function test2() {
        var_dump(bar);
        echo "Done\n";
    }
}

namespace {
    <<__EntryPoint>>
    function main() {
        require 'includes/global_bar.inc';
        require 'includes/foo_bar.inc';

        \test1();
        \test2();
    }
}
