<?hh


<<__EntryPoint>>
function main_printf_null() :mixed{
$format = "(\000)\n";

printf($format);

vprintf($format, varray[]);

echo sprintf($format);

echo vsprintf($format, varray[]);
}
