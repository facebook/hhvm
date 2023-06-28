<?hh

class C
{
///*
    public function __invoke($p)
:mixed    {
        echo "Inside " . __METHOD__ . " with arg $p\n";

        return "xxx";
    }
//*/
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $c = new C;
  var_dump(is_callable($c)); // returns TRUE is __invoke exists; otherwise, FALSE
  $r = $c(123);
  var_dump($r);
  $r = $c("Hello");
  var_dump($r);
}
