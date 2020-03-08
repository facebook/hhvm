<?hh // strict

namespace NS_Complex;

class Complex<T as num> {
  private T $real;
  private T $imag;

  public function __construct(T $real, T $imag) {
    $this->real = $real;
    $this->imag = $imag;
  }

  public function getReal(): T {
    return $this->real;
  }

  public function setReal(T $real): void {
    $this->real = $real;
  }

  public function getImag(): T {
    return $this->imag;
  }

  public function setImag(T $imag): void {
    $this->imag = $imag;
  }

  public static function add(Complex<T> $z1, Complex<T> $z2): Complex<T> {
    return new Complex($z1->real + $z2->real, $z1->imag + $z2->imag);
  }

  public static function subtract(Complex<T> $z1, Complex<T> $z2): Complex<T> {
    return new Complex($z1->real - $z2->real, $z1->imag - $z2->imag);
  }

  public function __toString(): string {
    if ($this->imag < 0.0) {                                                       
      return "(" . $this->real . " - " . (-$this->imag) . "i)";
    } else if (1.0/$this->imag == -INF) {
      return "(" . $this->real . " + " . 0.0 . "i)";
    } else {
      return "(" . $this->real . " + " . (+$this->imag) . "i)";
    }
  }
}
