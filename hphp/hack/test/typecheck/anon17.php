<?hh

function expects_vec_func_str(Vector<(function(): string)> $vec): void {}

function test(): void {
  $funcs = Vector { () ==> 1.0, () ==> 0, () ==> '' };

  expects_vec_func_str($funcs);
}
