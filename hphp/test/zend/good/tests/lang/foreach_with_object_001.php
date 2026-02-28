<?hh
class Test {
   public $a = vec[1,2,3,4,5]; // removed, crash too
   function c() :mixed{
      return new Test();
   }

}
<<__EntryPoint>> function main(): void {
$obj = new Test();
foreach ($obj->c()->a as $value) {
    print "$value\n";
}

echo "===DONE===\n";
}
