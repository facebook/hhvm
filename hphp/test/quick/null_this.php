<?php

class a {
  public $pub = 'public in a';
  function pub() {
    echo "Entering a::pub\n";
    var_dump($this->pub);
    echo "Leaving a::pub\n";
  }
}

class x {
  public $pub = 'public in x';
  function pub() {
    echo "Entering x::pub\n";
    a::pub();
    echo "Leaving x::pub\n";
  }
}

function main() {
  $a = new a;
  $x = new x;

  echo "Calling a->pub\n";
  $a->pub();
  echo "Calling x->pub\n";
  $x->pub();
}

echo "Calling main()\n";
main();
echo "Done\n";
