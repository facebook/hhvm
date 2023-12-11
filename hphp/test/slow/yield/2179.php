<?hh

trait DY {
  private $drc = dict[];
  public function dty($key) :AsyncGenerator<mixed,mixed,void>{
    $this->drc[$key] = true;
    yield (true);
  }
  public function edd($key) :mixed{
    if (array_key_exists($key, $this->drc)) {
      var_dump(true);
    }
   }
}
class C {
 use DY;
 }
class D extends C {
 }

<<__EntryPoint>>
function main_2179() :mixed{
$obj = new D;
foreach($obj->dty('foo') as $var) {
  var_dump($var);
}
$obj->edd('foo');
}
