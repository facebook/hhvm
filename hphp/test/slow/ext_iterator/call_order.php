<?hh

class A extends RecursiveDirectoryIterator {
  function __construct($a) {
    echo __FUNCTION__."\n";
    return parent::__construct($a);
  }
  function current() :mixed{
    echo __FUNCTION__."\n";
    return parent::current();
  }
  function next() :mixed{
    echo __FUNCTION__."\n";
    return parent::next();
  }
  function rewind() :mixed{
    echo __FUNCTION__."\n";
    return parent::rewind();
  }
}


<<__EntryPoint>>
function main_call_order() :mixed{
$a = new A(__DIR__.'/../../sample_dir/');
echo "done construct\n";
foreach ($a as $filename => $cur) {
}
}
