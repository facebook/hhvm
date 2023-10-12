<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public static function decryptParams <Tk as arraykey, Tv>(
  ): KeyedContainer<Tk, Tv> {
    return dict[];
  }

  private ?string $title;
  private bool $whatever = false;
  public function bar(Map<string, bool> $m): void {
    $m = C::decryptParams();
    // This is rejected by legacy type checker if we remove the non_null hack
    // from the checking of idx
    $this->title = idx($m, 'title');
    $this->whatever = (bool) idx($m, 'whatever', false);
  }
}
