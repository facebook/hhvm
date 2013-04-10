<?php

class TestParentClass
{
    public function method()
    {
        print_r('Parent method');
        print "\n";
    }
}

trait TestTrait
{
    public function method()
    {
        print_r('Trait method');
        print "\n";
    }
}

class TestChildClass extends TestParentClass
{
    use TestTrait
    {
        TestTrait::method as methodAlias;
        TestParentClass::method insteadof TestTrait;
    }
}
?>