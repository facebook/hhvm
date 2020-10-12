<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 0;
}

function basic_pipe(C $c): void {
  $c->a = $c->b |> $$;
}

function pc_pipe(C $c, Exception $e): void {
  $maybe_throw = (int $x) ==> {
    if ($x > 0) {
      throw $e;
    }
    return 0;
  };
  $write_a = $x ==> {
    $c->a = $x;
  };
  // Illegal! The PC during the write depends on B
  $maybe_throw($c->b) |> $write_a($$);
}

function chained_pipes(C $c): void {
  $ret_a = (int $_) ==> $c->a;
  $ret_b = (int $_) ==> $c->b;
  $c->a =
    0
    |> $ret_a($$) // $$ is Public
    |> $ret_b($$); // $$ is A
}
