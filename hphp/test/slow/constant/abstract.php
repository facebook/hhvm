<?hh
interface i1 {
  abstract const con;
}
interface i2 {
  abstract const con;
}
class a implements i1, i2 {
  const con = 42;
  public function bar() :mixed{
    echo "Fail\n";
  }
  final public function main() :mixed{
    return $this->bar();
  }
}
class b extends a {
  public function bar() :mixed{
    echo "Pass\n";
  }
}

<<__EntryPoint>>
function main_abstract() :mixed{
(new b)->main();
}
