<?hh

namespace test;
const TEST = 11;

class foo {
    public function xyz() {
    }
}

interface baz {
}

function bar() {
}

<<__EntryPoint>> function main(): void {
\var_dump(\interface_exists('\test\baz'));
\var_dump(\function_exists('\test\bar'));
\var_dump(\constant('\test\TEST'));
\var_dump(\defined('\test\TEST'));
\var_dump(\defined('TEST'));
}
