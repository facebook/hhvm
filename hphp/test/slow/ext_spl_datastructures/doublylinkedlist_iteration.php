<?hh

<<__EntryPoint>>
function main_doublylinkedlist_iteration() :mixed{
$stack = new SplStack();

$stack->push("var1");
$stack->push("var2");
$stack->push("var3");

foreach ($stack as $var) {
    echo $var . "\n";
}
}
