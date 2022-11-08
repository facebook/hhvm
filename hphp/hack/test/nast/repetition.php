<?hh

function repetition_fun_param(int $x, int $x): void {}

function repetition_fun_param_placeholder(int $_, int $_): void {}

class RepetitionClassTparam<T, T> {}

function repetition_fun_tparam<T, T>(): void {}

class ABCD {
  public function repetition_method_param<T, T>(): void {}
}
