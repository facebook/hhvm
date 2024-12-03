<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/**
 * You should probably use TMemoryBuffer instead.
 *
 * A memory buffer is a transport that simply reads from and writes to an
 * in-memory string buffer. TChunkedMemoryBuffer is like TMemoryBuffer but
 * uses multiple strings to get around HHVM's internal string size limit.
 */
<<Oncalls('thrift')>>
final class TChunkedMemoryBuffer
  extends TWritePropsTransport
  implements IThriftBufferedTransport {

  private ?int $length = null;
  private vec<string> $bufs = vec[""];
  private int $index = 0;

  public function __construct(private int $chunkSize)[] {}

  public function setChunks(vec<string> $bufs)[write_props]: void {
    // THIS HAS NOT BEEN TESTED PARTICULARLY WELL PROCEED AT YOUR OWN RISK
    $this->bufs = $bufs;
    $this->length = null;
    $this->length();
  }

  <<__Override>>
  public function isOpen()[]: bool {
    return true;
  }

  <<__Override>>
  public function open()[]: void {}

  <<__Override>>
  public function close()[]: void {}

  public function length()[write_props]: int {
    if ($this->length === null) {
      $length = 0;
      foreach ($this->bufs as $buf) {
        $length += Str\length($buf);
      }
      $this->length = $length;
    }
    return $this->length;
  }

  <<__Override>>
  public function write(string $buf)[write_props]: void {
    $wrote = 0;
    while ($wrote < Str\length($buf)) {
      $idx = C\count($this->bufs) - 1;
      $cap = $this->chunkSize - Str\length($this->bufs[$idx]);
      if ($cap === 0) {
        $this->bufs[] = "";
        continue;
      }
      $this->bufs[$idx] .= Str\slice($buf, $wrote, $cap);
      $wrote += Math\minva($cap, Str\length($buf));
    }
    $this->length = null; // reset length
  }

  <<__Override>>
  public function read(int $len)[write_props]: string {
    // THIS HAS NOT BEEN TESTED PARTICULARLY WELL PROCEED AT YOUR OWN RISK
    $ret = $this->peek($len);
    $this->index += $len;
    return $ret;
  }

  public function peek(int $len, int $start = 0)[]: string {
    // THIS HAS NOT BEEN TESTED PARTICULARLY WELL PROCEED AT YOUR OWN RISK
    $index = $this->index + $start;
    $current_chunk = Math\int_div($index, $this->chunkSize);
    $ret = vec[];
    $chunk_idx = $index - $current_chunk * $this->chunkSize;
    $remaining = $this->chunkSize - $chunk_idx;

    if ($remaining > $len) {
      $ret = Str\slice($this->bufs[$current_chunk], $chunk_idx, $len);
      return $ret;
    } else {
      $len -= $remaining;
      $index += $remaining;
      $ret[] = Str\slice($this->bufs[$current_chunk], $chunk_idx);
      $current_chunk++;
    }

    while ($len > $this->chunkSize) {
      $len -= $this->chunkSize;
      $ret[] = $this->bufs[$current_chunk];
      $index += $this->chunkSize;
      $current_chunk++;
    }

    $ret[] = Str\slice($this->bufs[$current_chunk], 0, $len);
    return Str\join($ret, "");
  }

  public function getChunks()[]: vec<string> {
    return $this->bufs;
  }

  public function resetBuffer()[write_props]: void {
    $this->bufs = vec[""];
    $this->length = null;
  }
}
