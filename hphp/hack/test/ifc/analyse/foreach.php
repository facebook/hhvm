<?hh

<<__InferFlows>>
function dict_lit_flow(): void {
  foreach (dict<string,int>[] as $key => $value) { }
}

<<__InferFlows>>
function dict_var_flow(dict<string,int> $dict): void {
  foreach ($dict as $key => $value) { }
}

<<__InferFlows>>
function vec_lit_flow(): void {
  foreach (vec<int>[] as $value) { }
}

<<__InferFlows>>
function vec_var_flow(vec<int> $vec): void {
  foreach ($vec as $value) { }
}

<<__InferFlows>>
function keyset_lit_flow(): void {
  foreach (keyset<int>[] as $value) { }
}

<<__InferFlows>>
function keyset_var_flow(keyset<int> $keyset): void {
  foreach ($keyset as $value) { }
}
