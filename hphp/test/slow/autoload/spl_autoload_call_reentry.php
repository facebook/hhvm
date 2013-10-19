<?php

class Autoloaders
{
  public function autoload1($class)
  {
    echo "autoload1\n";
  }

  public function autoload2($class)
  {
    echo "autoload2\n";

    spl_autoload_unregister(array($this, 'autoload2'));
    spl_autoload_register(array($this, 'autoload2'));

    spl_autoload_call($class);
  }

  public function autoload3($class)
  {
    echo "autoload3\n";
    exit();
  }
}

$autoloaders = new Autoloaders();
spl_autoload_register(array($autoloaders, 'autoload1'));
spl_autoload_register(array($autoloaders, 'autoload2'));
spl_autoload_register(array($autoloaders, 'autoload3'));
$foo = new NotRealAutoloadedClass();
