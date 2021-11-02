<?hh //partial

class CFilter {
  public function cfilterMethod(): void {}
}

interface IFilter {
  public function ifilterMethod(): void;
}

trait TFilter {
  public function tfilterMethod(): void {}
}

class Filter extends CFilter implements IFilter {
  use TFilter;

  public function cfilterMethod(): void {}
  public function ifilterMethod(): void {}
  public function tfilterMethod(): void {}
}
