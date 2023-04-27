<?hh

/** Doc for IBase1 */
interface IBase1 {}
interface IBase2 {}

interface I extends IBase1, IBase2 {
    abstract const type TBoth as IBase1 as IBase2;
//                                   ^ hover-at-caret
}
