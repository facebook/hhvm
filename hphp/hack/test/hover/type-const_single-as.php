<?hh

interface I {
    abstract const type T1 as num;
    public function use_num(I::T1 $_): void {}
//                                ^ hover-at-caret
}
