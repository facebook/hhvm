<?hh

class C {
      public int $x = 0;
}

function un_readonly<T>(readonly T $x, T $y = readonly $x): T {
    return $y;
}

<<__EntryPoint>>
  function main(): void {
    $c = readonly new C();
      $c = un_readonly($c);
          $c->x = 3;  // should fatal
              echo $c->x; // 3
  }
