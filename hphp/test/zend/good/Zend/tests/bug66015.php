<?hh
class Test
{
   const FIRST = 1;
   const SECOND = 2;
   const THIRD = 3;

   protected static $array = dict[
       self::FIRST => 'first',
       self::SECOND => 'second',
       self::THIRD => 'third'
   ];

   public function __construct()
   {
       var_export(self::$array);
   }
}
<<__EntryPoint>> function main(): void {
$test = new Test();
}
