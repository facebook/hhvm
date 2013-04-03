<?php
class DestructableObject
{
        public function __destruct()
        {
                DestructableObject::__destruct();
        }
}
class DestructorCreator
{
        public function __destruct()
        {
                $this->test = new DestructableObject;
        }
}
class Test
{
        public static $mystatic;
}
$x = new Test();
Test::$mystatic = new DestructorCreator();