<?hh

// Testing ob_start, chunking and the debugger since the debugger actually
// swaps buffers sometimes.
class Foo {
  function method() {
    $other = $this; // breakpoint is set here
  }
}


// 13 chunk, random
<<__EntryPoint>>
function main_print_this_ob() {
ob_start(function ($s) { return 'ob: ' . $s; }, 13);
echo "I am going ";
echo "to see if this ";
echo "chunking works well\n";

$object = new Foo;
$object->prop = "Hello\n";

$object->method(); // breakpoint will get hit on this call

$object->prop2 = "\tThere";
$object->method(); // and here

echo "I sure hope it does; ";
echo "otherwise we debug!\n";

ob_end_flush();
}
