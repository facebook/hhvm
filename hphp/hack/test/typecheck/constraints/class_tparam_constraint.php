<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface ITP {}
class CTS<Tt> {}
final class FTS<T as ITP> extends CTS<T> {
  private ?CTS<T> $server;

  public function setupTS(CTS<T> $server): this {
    $this->server = $server;

    return $this;
  }
}
