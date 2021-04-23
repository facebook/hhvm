<?hh

abstract class C {
  abstract const ctx C0 = [];
  abstract const ctx C1 = [policied_shallow];
  abstract const ctx C2 = [policied_shallow, rx_shallow];

  public abstract function f0()[]: void;
  public abstract function f1()[policied_shallow]: void;
  public abstract function f2()[policied_shallow, rx_shallow] : void;
}
