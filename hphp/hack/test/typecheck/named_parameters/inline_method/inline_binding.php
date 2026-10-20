<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineBindingTypecheck {
  private function names(named string $param_a, named int $param_b): string {
    return $param_a.$param_b;
  }

  private function mixed(string $first, named int $scale, named string $last): string {
    return $first.$scale.$last;
  }

  private function defaults(int $first = 1, named int $choice = 2, int $last = 3): int {
    return $first + $choice + $last;
  }

  private function collection(vec<int> $values = vec[1, 2]): int {
    return count($values);
  }

  private function variadic(int $head, int ...$rest, named int $scale = 1): int {
    return $head + count($rest) * $scale;
  }

  private function named_variadic(int ...$rest, named string...): int {
    return count($rest);
  }

  public function caller(): void {
    $x = 'previous';
    $a = $this->names(param_b=2, param_a='hello');
    $b = $this->mixed('first', last=$x, scale=3);
    $c = $this->defaults(choice=5, 6);
    $d = $this->defaults(1, 4);
    $e = $this->collection();
    $f = $this->variadic(1, scale=2, 3, 4);
    $g = $this->variadic(1);
    $h = $this->named_variadic(1, label='hello', 2, more='world');
    $i = $this->named_variadic();
  }
}
