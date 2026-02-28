<?hh


<<__EntryPoint>>
function main_printf_null() :mixed{
$format = "(\000)\n";

printf($format);

vprintf($format, vec[]);

echo sprintf($format);

echo vsprintf($format, vec[]);
}
