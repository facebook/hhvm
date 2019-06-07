<?hh

// All constructors should be registered as such

trait TConstructor {
    public function constructor() {
        echo "ctor executed\n";
    }
}

class NewConstructor {
    use TConstructor {
        constructor as __construct;
    }
}
<<__EntryPoint>> function main() {
echo "New constructor: ";
$o = new NewConstructor;
}
