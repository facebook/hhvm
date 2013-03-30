<?php
class test
{
    private static $instance = null;
    private $myname = '';
    
    private function __construct( $value = '' ) 
    {
        echo "New class $value created \n";
        $this -> myname = $value;
    }
    private function __clone() {}
    static public function getInstance()
    {
        if ( self::$instance == null )
        {
            self::$instance = new test('Singleton1');
        }
        else {
            echo "Using old class " . self::$instance -> myname . "\n";
        }
        return self::$instance;
    }
    static public function getInstance2()
    {
        static $instance2 = null;
        if ( $instance2 == null )
        {
            $instance2 = new test('Singleton2');
        }
        else {
            echo "Using old class " . $instance2 -> myname . "\n";
        }
        return $instance2;
    }
    public function __destruct() 
    {
        if ( defined('SCRIPT_END') )
        {
            echo "Class " . $this -> myname . " destroyed at script end\n";
        } else {
            echo "Class " . $this -> myname . " destroyed beforce script end\n";
        }
    }
}    
echo "Try static instance inside class :\n";
$getCopyofSingleton    = test::getInstance();
$getCopyofSingleton    = null;
$getCopyofSingleton    = &test::getInstance();
$getCopyofSingleton    = null;
$getCopyofSingleton    = test::getInstance();
echo "Try static instance inside function :\n";
$getCopyofSingleton2   = test::getInstance2();
$getCopyofSingleton2   = null;
$getCopyofSingleton2   = &test::getInstance2();
$getCopyofSingleton2   = null;
$getCopyofSingleton2   = test::getInstance2();

define('SCRIPT_END',1);
?>