<?hh

class test {
  public int $one = 1;
  private int $priv = 2;
  public int $two = 2;
  protected int $prot = 3;
}


<<__EntryPoint>>
function main_array_public() :void{
$ai = new ArrayIterator(new test());

foreach ($ai as $key=>$val) {
  echo $key;
}
}
