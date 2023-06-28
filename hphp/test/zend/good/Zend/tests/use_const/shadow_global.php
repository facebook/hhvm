<?hh

namespace {
    function test1() :mixed{
        var_dump(bar);
    }
}

namespace {
    use const foo\bar;
    
    function test2() :mixed{
        var_dump(bar);
        echo "Done\n";
    }
}

namespace {
    <<__EntryPoint>>
    function main() :mixed{
        require 'includes/global_bar.inc';
        require 'includes/foo_bar.inc';

        \test1();
        \test2();
    }
}
