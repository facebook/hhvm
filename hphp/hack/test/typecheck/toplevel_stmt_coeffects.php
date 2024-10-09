<?hh

class Box {public function __construct(public int $value){}}

(() ==> {
  $x = new Box(1);
  $x->value = 2;
})();
