<?hh
function foo1() :mixed{
    try {
        throw new Exception("not catch");
        return true;
    } finally {
        try {
            throw new Exception("catched");
        } catch (Exception $e) {
        }
    }
}

function foo2() :mixed{
    try  {
        try {
            throw new Exception("catched");
            return true;
        } finally {
            try {
                throw new Exception("catched");
            } catch (Exception $e) {
            }
        }
    } catch (Exception $e) {
    }
}

function foo3() :mixed{
    try {
        throw new Exception("not catched");
        return true;
    } finally {
        try {
            throw new NotExists();
        } catch (Exception $e) {
        }
    }
}

<<__EntryPoint>> function main(): void {
try {
    $foo = foo1();
    var_dump($foo);
} catch (Exception $e) {
    do {
        var_dump($e->getMessage());
    } while ($e = $e->getPrevious());
}

$foo = foo2();
var_dump($foo);

$bar = foo3();
}
