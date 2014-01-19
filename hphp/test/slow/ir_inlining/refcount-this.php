<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function ident($a) { return $a; }

class BMTR {
  public function init() {
    $this->registerToolkit(ident($this));
  }

  private function registerToolkit($derp) {
    // Do some crap, to block inlining
    var_dump($derp);
    var_dump('a' . 'b');
  }
}

(new BMTR)->init();
