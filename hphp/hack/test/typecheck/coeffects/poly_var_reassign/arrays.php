<?hh

function append(vec<int> $v, Vector<int> $vm)[$v::C, $vm::C, local]: void {
  $v[] = 0; // error bc modeled in typechecker as $v = <new_expr>;
  $vm[] = 0; // does not update expr id
}

function update(vec<int> $v, Vector<int> $vm)[$v::C, $vm::C, local]: void {
  $v[0] = 0; // error
  $vm[0] = 0;
}
