<?hh
class MyAutoloader {
    function __construct($directory_to_use) {}
    function autoload($class_name) {
        // code to autoload based on directory
    }
}
<<__EntryPoint>> function main(): void {
$autloader1 = new MyAutoloader('dir1');
spl_autoload_register(varray[$autloader1, 'autoload']);

$autloader2 = new MyAutoloader('dir2');
spl_autoload_register(varray[$autloader2, 'autoload']);

print_r(spl_autoload_functions());
echo "===DONE===\n";
}
