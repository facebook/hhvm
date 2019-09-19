<?hh

class C {
  public $publicFoo;
  protected $protectedFoo;
  private $privateFoo;
  public static $staticFoo;
}
<<__EntryPoint>>
function main_entry(): void {
  //New instance of class C - defined below
  $rc = new ReflectionClass("C");

  //Check if C has public property publicFoo
  var_dump($rc->hasProperty('publicFoo'));

  //Check if C has protected property protectedFoo
  var_dump($rc->hasProperty('protectedFoo'));

  //Check if C has private property privateFoo
  var_dump($rc->hasProperty('privateFoo'));

  //Check if C has static property staticFoo
  var_dump($rc->hasProperty('staticFoo'));

  //C should not have property bar
  var_dump($rc->hasProperty('bar'));
}
