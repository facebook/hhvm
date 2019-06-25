<?hh

class MyAutoLoader {

        static function autoLoad($className) {
            echo __METHOD__ . "($className)\n";
        }
}
<<__EntryPoint>> function main(): void {
spl_autoload_register('MyAutoLoader::autoLoad');

var_dump(spl_autoload_functions());

// check
var_dump(class_exists("TestClass", true));

echo "===DONE===\n";
}
