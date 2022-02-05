<?hh

abstract class C {
  abstract const ctx C0 = [];
  abstract const ctx C1 = [zoned_shallow];
  abstract const ctx C2 = [zoned_shallow, rx_shallow];

  public abstract function f0()[]: void;
  public abstract function f1()[zoned_shallow]: void;
  public abstract function f2()[zoned_shallow, rx_shallow] : void;
}
