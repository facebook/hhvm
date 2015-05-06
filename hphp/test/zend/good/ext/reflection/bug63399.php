<?php
trait Trait1 {
        public function run() {}
        public function say() {}
}

trait Trait2 {
        public function run() {}
        public function say() {}
}

class MyClass
{
    use Trait1, Trait2 {
        Trait1::run as execute;
        Trait1::say insteadof Trait2;
        Trait2::run insteadof Trait1;
        Trait2::say as talk;
    }
}

$ref = new ReflectionClass('MyClass');

print_r($ref->getTraitAliases());
print_r($ref->getTraits());

?>
