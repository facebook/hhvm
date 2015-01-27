<?php
file_put_contents(dirname(__FILE__)."/bug63741.tmp.php",
<<<'EOT'
<?php
if (isset($autoloading))
{
    class ClassToLoad
    {
        static function func ()
        {
            print "OK!\n";
        }
    }
    return;
}
else
{
    class autoloader
    {
        static function autoload($classname)
        {
            print "autoloading...\n";
            $autoloading = true;
            include __FILE__;
        }
    }

    spl_autoload_register(["autoloader", "autoload"]);

    function start()
    {
        ClassToLoad::func();
    }

    start();
}
?>
EOT
);

include dirname(__FILE__)."/bug63741.tmp.php";
?>
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__)."/bug63741.tmp.php"); ?>