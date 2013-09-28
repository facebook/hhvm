<?hh

class Yeah {
  public function __construct(
    <<Data>> public int $x,
    <<Data>> public int $y,
    <<Data>> string $s,
    <<Data>> private ?Point $p) {}
}

$p = new Yeah(10, 10, 'f', null);
var_dump($p->x . ', ' . $p->y);

