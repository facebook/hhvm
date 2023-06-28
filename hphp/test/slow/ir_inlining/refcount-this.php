<?hh


function ident($a) :mixed{ return $a; }

class BMTR {
  public function init() :mixed{
    $this->registerToolkit(ident($this));
  }

  private function registerToolkit($derp) :mixed{
    // Do some crap, to block inlining
    var_dump($derp);
    var_dump('a' . 'b');
  }
}


<<__EntryPoint>>
function main_refcount_this() :mixed{
(new BMTR)->init();
}
