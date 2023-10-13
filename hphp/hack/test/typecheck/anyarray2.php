<?hh

function fun(dict<string, vec<KeyedContainer<string, string>>> $in): void {
  $stack = Vector {dict['k' => 'v']};
  while (!$stack->isEmpty()) {
    $row = $stack->pop();
    $r = idx($in, $row['k'], vec[]);
    $stack->addAll($r);
  }
}
