<?hh

namespace {
    require 'includes/global_bar.php';
    require 'includes/foo_bar.php';
}

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
        \test1();
        \test2();
    }
}
