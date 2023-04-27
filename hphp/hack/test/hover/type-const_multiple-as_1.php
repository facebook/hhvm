<?hh

interface I extends IBase1, IBase2 {
    abstract const type TFstWins as int as arraykey;
    public function use_num(I::TFstWins $_): void {}
//                                      ^ hover-at-caret
}
