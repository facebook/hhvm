<?hh

<<__EntryPoint>>
function main(): void {
    $a = new XMLReader();
    $b = new DOMNode();
    $a->expand($b);
    $a->expand($a);
}
