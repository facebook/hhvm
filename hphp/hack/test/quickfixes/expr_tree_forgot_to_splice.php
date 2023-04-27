<?hh

function f(int $num): void {
    DSL`$num + 1`;

    DSL`{
      $NUM = 1;
      return $nuM + 1;
    }`;
}
