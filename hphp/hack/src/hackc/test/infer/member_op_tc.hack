class C {
  public int $foo = 42;
}

function ret_c(): C {
  return new C();
}

function ret_vec(): vec<int> {
  return vec[];
}

function ret_int(): int {
  return 42;
}

function ret_str(): string {
  return "hello";
}
