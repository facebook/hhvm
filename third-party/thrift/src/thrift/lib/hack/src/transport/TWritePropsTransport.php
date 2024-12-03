<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

abstract class TWritePropsTransport extends TTransport {
  <<__Override>>
  public abstract function open()[write_props]: void;

  <<__Override>>
  public abstract function close()[write_props]: void;

  <<__Override>>
  public abstract function read(int $len)[write_props]: string;

  <<__Override>>
  public abstract function write(string $buf)[write_props]: void;

  <<__Override>>
  public function readAll(int $len)[write_props]: string {
    // return $this->read($len);
    $data = '';
    for ($got = Str\length($data); $got < $len; $got = Str\length($data)) {
      $data .= $this->read($len - $got);
    }
    return $data;
  }

  <<__Override>>
  public function flush()[write_props]: void {}

  <<__Override>>
  public function onewayFlush()[write_props]: void {
    // Default to flush()
    $this->flush();
  }
}
