<?hh

// should not throw syntax errors

use Foo\Bar     \{ A };

use Foo\Bar\    { B };

use Foo\Bar
\{
    C
};

use Foo\Bar\
{
    D
};
<<__EntryPoint>>
function entrypoint_ns_093(): void {

  echo "\nDone\n";
}
