<?hh

class Autoloaders
{
  public function autoload1($class)
  {
    echo "autoload1\n";
  }

  public function autoload2($class)
  {
    echo "autoload2\n";

    spl_autoload_unregister(varray[$this, 'autoload2']);
    spl_autoload_register(varray[$this, 'autoload2']);

    spl_autoload_call($class);
  }

  public function autoload3($class)
  {
    echo "autoload3\n";
    exit();
  }
}


<<__EntryPoint>>
function main_spl_autoload_call_reentry() {
$autoloaders = new Autoloaders();
spl_autoload_register(varray[$autoloaders, 'autoload1']);
spl_autoload_register(varray[$autoloaders, 'autoload2']);
spl_autoload_register(varray[$autoloaders, 'autoload3']);
$foo = new NotRealAutoloadedClass();
}
