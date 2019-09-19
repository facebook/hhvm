<?hh

function test1(...$args) {
    var_dump(count($args));
}

function test2($a, ...$args) {
    var_dump(1 + count($args));
}

function test3($a, $b, ...$args) {
    var_dump(2 + count($args));
}

class test {
    static function test1($a, ...$args) {
        var_dump(1 + count($args));
    }
}
<<__EntryPoint>> function main(): void {
test1();
test2(1);
try { test2(); } catch (Exception $e) { var_dump($e->getMessage()); }
test3(1,2);

call_user_func(fun("test1"));
try { call_user_func(fun("test3"), 1); } catch (Exception $e) { var_dump($e->getMessage()); }
call_user_func(fun("test3"), 1, 2);

test::test1(1);

echo "Done\n";
}
