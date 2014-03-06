<?php

namespace __SystemLib {
  function register_default_stream_filters() {
    \stream_filter_register('zlib.deflate', '__SystemLib\DeflateStreamFilter');
    \stream_filter_register('zlib.inflate', '__SystemLib\InflateStreamFilter');
  }

  class DeflateStreamFilter extends \php_user_filter {
    private int $level = -1; // zlib default
    private string $buffer = '';

    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        $this->buffer .= $bucket->data;
      }

      if ($closing) {
        if (is_int($this->params)) {
          $this->level = $this->params;
        }
        if (is_array($this->params) && isset($this->params['level'])) {
          $this->level = $this->params['level'];
        }
        stream_bucket_append(
          $out,
          stream_bucket_new(
            $this->stream,
            gzdeflate($this->buffer, $this->level)
          )
        );
      }

      return PSFS_PASS_ON;
    }
  }

  class InflateStreamFilter extends \php_user_filter {
    public function onCreate(): bool {
      $this->impl = new \__SystemLib_ChunkedInflator();
      return true;
    }

    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        if ($this->impl->eof()) {
          return PSFS_ERR_FATAL;
        }
        $this_chunk = $this->impl->inflateChunk($bucket->data);
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream,$this_chunk)
        );
      }

      if ($this->impl->eof()) {
        return PSFS_PASS_ON;
      } else {
        return PSFS_FEED_ME;
      }
    }
  }
}
