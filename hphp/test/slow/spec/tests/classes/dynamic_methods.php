<?hh

class Widget
{
    public function iDoit()
    {
        echo "Inside " . __METHOD__ . "\n";
        return 99;
    }

    public static function sDoit()
    {
        echo "Inside " . __METHOD__ . "\n";
        return 88;
    }
///*
    public function __call($name, $arguments)
    {
        echo "Calling instance method >$name<\n";
        var_dump($arguments);

        return 987;
    }
//*/
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $obj = new Widget;
  $v = $obj->iDoit();
  $obj->__call('iDoit', varray[]);

  $v = $obj->iMethod(10, TRUE, "abc");
  var_dump($v);
  $obj->__call('iMethod', varray[10, TRUE, "abc"]);
  $obj->__call('123#$%', varray[]);

  $v = Widget::sDoit();
}
