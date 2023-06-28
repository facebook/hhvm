<?hh

function foo() :mixed{ }
<<__EntryPoint>> function main(): void {
$data = varray[
    foo<>,
    strtolower<>,
    1,
    1.1231
];

foreach ($data as $callback) {
    readline_completion_function($callback);
}
}
