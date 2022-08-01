<?hh

interface IBase1 {}
interface IBase2 {}

interface I extends IBase1, IBase2 {
    abstract const type TBoth as IBase1 as IBase2;
    public function use_num(I::TBoth $_): void {}
//                                   ^ hover-at-caret
}
