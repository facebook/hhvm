<?hh

class C { public static $storage; }

<<__EntryPoint>>
function bug60768() :mixed{
ob_start(function($buffer) { C::$storage .= $buffer; }, 20);

echo str_repeat("0", 20); // fill in the buffer

for($i = 0; $i < 10; $i++) {
    echo str_pad((string)$i, 9, ' ', STR_PAD_LEFT) . "\n"; // full buffer dumped every time
}

ob_end_flush();

printf("Output size: %d, expected %d\n", strlen(C::$storage), 20 + 10 * 10);
}
