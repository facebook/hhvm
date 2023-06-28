<?hh

class Club {
  protected $app_id = 0;
}

class Glub extends Club {
  public function go() :mixed{
    var_dump(property_exists($this, 'app_id'));
  }
}
<<__EntryPoint>> function main(): void {
$g = new Glub();
$g->go();
var_dump(property_exists($g, 'app_id'));
}
