<?hh
namespace test;
class inner {}
<<__EntryPoint>> function main(): void {
$inner = new \test\inner();

echo "autoload == true:\n";
\var_dump(\class_exists('\test\inner', true));
echo "autoload == false:\n";
\var_dump(\class_exists('\test\inner', true));
}
