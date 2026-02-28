<?hh

interface I {
    abstract const type T1 as int as float;  // intentional gibberish
    public function use_num(I::T1 $_): void {}
//                                ^ hover-at-caret
}
