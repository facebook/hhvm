<?hh

interface IVCB {}
interface IEB {}
interface EMVB<+Tv> {}

interface IESO<TVC as IVCB, TV as EMVB<TVC>> {}

abstract class ESOB implements IESO<this::TVC, this::TV> {

  abstract const type TVC as IVCB;
  abstract const type TV as EMVB<this::TVC>;
}
