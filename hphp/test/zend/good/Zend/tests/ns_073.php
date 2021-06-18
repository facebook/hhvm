<?hh

namespace foo;
<<__EntryPoint>> function main(): void {
$x = function (\stdClass $x = NULL) {
    \var_dump($x);
};

$x(NULL);
$x(new \stdClass);
}
