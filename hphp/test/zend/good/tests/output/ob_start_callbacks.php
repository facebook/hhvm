<?hh

class foo {
    static function out($foo) {
        return strtoupper($foo);
    }
}

function my_strtoupper($foo, $bar) {
    return strtoupper($foo);
}

// Closure in variable
<<__EntryPoint>> function main(): void {
$a = function ($s) { return strtoupper($s); };
ob_start($a);
echo 'closure in variable', "\n";
ob_end_flush();

// Object (array) in variable
$a = varray['foo', 'out'];
ob_start($a);
echo 'object in variable', "\n";
ob_end_flush();

// Object with static array
ob_start(varray['foo', 'out']);
echo 'object via static array', "\n";
ob_end_flush();

$a = 'my_strtoupper';
ob_start($a);
echo 'function via variable', "\n";
ob_end_flush();
}
