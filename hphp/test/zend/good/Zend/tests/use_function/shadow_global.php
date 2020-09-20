<?hh

namespace {

}

namespace {
    function test1() {
        var_dump(bar());
    }
}

namespace {
    use function foo\bar;
    function test2() {
        var_dump(bar());
        echo "Done\n";
    }
}

namespace {
    <<__EntryPoint>>
    function main() {
        require 'includes/global_bar.php';
        require 'includes/foo_bar.php';

        test1();
        test2();
    }
}
