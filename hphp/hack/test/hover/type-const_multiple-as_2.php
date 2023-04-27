<?hh

interface I extends IBase1, IBase2 {
    abstract const type TSndWins as int as arraykey;
    public function use_num(I::TSndWins $_): void {}
//                                      ^ hover-at-caret
}
