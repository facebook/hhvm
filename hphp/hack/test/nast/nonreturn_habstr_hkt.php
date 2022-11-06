<?hh

class ABC<T1<T2>> {
  public function nonreturn_habstr_void_hkt(T1<void> $x): void {}

  public function nonreturn_habstr_noreturn_hkt(T1<noreturn> $x): void {}
}
