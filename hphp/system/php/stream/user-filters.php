<?php

class php_user_filter {
  public function __construct(
    public resource $stream,
    public string $filtername,
    public $params
  ) {
  }

  public function filter(
    resource $in,
    resource $out,
    int &$consumed,
    bool $closed,
  ): int {
    return PSFS_ERR_FATAL;
  }

  public function onClose(): void {
  }

  public function onCreate(): ?bool {
    // Docs say bool, but Zend's default implementation returns null
    return null;
  }
}

function stream_bucket_new(resource $stream, string $buffer) {
  return new __SystemLib\StreamFilterBucket($buffer);
}

namespace __SystemLib {
  class StreamFilterBucket {
    public string $data;
    public int $dataLen;

    public function __construct(
      string $data,
      int $dataLen = 0,
    ) {
      if ($dataLen <= 0) {
        $dataLen = strlen($data);
      }
      $this->data = $data;
      $this->dataLen = $dataLen;
    }

    public function __toString() {
      if ($this->dataLen >= 0) {
        $len = min($this->dataLen, strlen($this->data));
        if ($len == 0) {
          // substr returns false on empty strings
          return '';
        } else {
          return substr($this->data, 0, $len);
        }
      } else {
        return $this->data;
      }
    }
  }
}
