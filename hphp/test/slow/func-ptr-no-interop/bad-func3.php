<?hh

<<__EntryPoint>>
function main() {
    $a = HH\fun('foo$inout');
    var_dump($a(1));
}
