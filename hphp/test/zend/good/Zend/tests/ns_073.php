<?hh

namespace foo;
<<__EntryPoint>> function main(): void {
$x = function (\stdclass $x = NULL) {
    \var_dump($x);
};

$x(NULL);
$x(new \stdclass);
}
