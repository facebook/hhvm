<?hh

class Callbacks {
  public $count = 0;
  <<__DynamicallyCallable>> public function filter($n) :mixed{
    $this->count++;
    return $n % 2;
  }

  public static $scount = 0;
  <<__DynamicallyCallable>> public static function sfilter($n) :mixed{
    self::$scount++;
    return $n % 3;
  }
}

<<__EntryPoint>> function main(): void {
  $a = vec[1,2,3,4,5,6,7,8];
  $cb = new Callbacks();
  var_dump(array_filter($a, vec[$cb, 'filter']));
  echo $cb->count . " times\n";

  var_dump(array_filter($a, vec['Callbacks', 'sfilter']));
  echo Callbacks::$scount . " times\n";
}
