<?php

namespace __SystemLib {
  function register_default_stream_filters() {
    \stream_filter_register('zlib.deflate', '__SystemLib\DeflateStreamFilter');
    \stream_filter_register('zlib.inflate', '__SystemLib\InflateStreamFilter');
    \stream_filter_register(
      'string.rot13',
      '__SystemLib\StringRot13StreamFilter'
    );
    \stream_filter_Register(
      'string.toupper',
      '__SystemLib\StringToUpperStreamFilter'
    );
    \stream_filter_Register(
      'string.tolower',
      '__SystemLib\StringToLowerStreamFilter'
    );
    \stream_filter_Register(
      'convert.iconv.*',
      '__SystemLib\ConvertIconvFilter'
    );
    \stream_filter_Register(
      'convert.*',
      '__SystemLib\ConvertFilter'
    );
    \stream_filter_Register(
      'bzip2.*',
      '__SystemLib\Bzip2Filter'
    );
  }

  class ConvertIconvFilter extends \php_user_filter {
    private $fromEncoding;
    private $toEncoding;

    public function onCreate(): bool {
      /* strip out prefix "convert.iconv." */
      $filter = substr($this->filtername, 14);
      if (false === strpos($filter, '/')) {
        return false;
      }
      $encodingPair = explode('/', $filter, 2);
      $this->fromEncoding = strtolower($encodingPair[0]);
      $this->toEncoding = strtolower($encodingPair[1]);
      return true;
    }

    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        $convertedData = iconv(
          $this->fromEncoding,
          $this->toEncoding,
          $bucket->data
        );
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream, $convertedData)
        );
      }
      return \PSFS_PASS_ON;
    }
  }

  class ConvertFilter extends \php_user_filter {
    private $filterFunction;

    public function onCreate(): bool {
      /* strip out prefix "convert." */
      $filterName = substr($this->filtername, 8);
      switch ($filterName) {
        case 'base64-encode':
        case 'base64-decode':
        case 'quoted-printable-encode':
        case 'quoted-printable-decode':
          $this->filterFunction = str_replace('-', '_', $filterName);
          break;
        default:
          return false;
          break;
      }
      return true;
    }

    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream,
                            call_user_func($this->filterFunction,
                                           $bucket->data)
                            )
        );
      }
      return \PSFS_PASS_ON;
    }
  }

  class Bzip2Filter extends \php_user_filter {
    const DEFAULT_BLOCKS = 4;
    const DEFAULT_WORK   = 0;

    private $filterFunction;

    public function onCreate(): bool {
      /* strip out prefix "bizp2." */
      $filterName = substr($this->filtername, 6);
      switch ($filterName) {
        case 'compress':
          if (is_int($this->params)) {
            $blocks = $this->params;
            $work = self::DEFAULT_WORK;
          } else if (is_array($this->params)) {
            $blocks = $this->params['blocks'] ?? self::DEFAULT_BLOCKS;
            $work = $this->params['work'] ?? self::DEFAULT_WORK;
          } else {
            $blocks = self::DEFAULT_BLOCKS;
            $work = self::DEFAULT_WORK;
          }
          $this->filterFunction = function($data) use($blocks, $work) {
            return bzcompress($data, $blocks, $work);
          };
          break;
        case 'decompress':
          if (is_bool($this->params)) {
            $small = $this->params;
          } else if (is_array($this->params)) {
            $small = $this->params['small'] ?? false;
          } else {
            $small = false;
          }
          $this->filterFunction = function($data) use($small) {
            return bzdecompress($data, $small);
          };
          break;
        default:
          return false;
          break;
      }
      return true;
    }

    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream,
                            call_user_func($this->filterFunction,
                                           $bucket->data)
                            )
        );
      }
      return \PSFS_PASS_ON;
    }
  }

  class DeflateStreamFilter extends \php_user_filter {
    private $level = -1; // zlib default
    private $buffer = '';

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

      return \PSFS_PASS_ON;
    }
  }

  class InflateStreamFilter extends \php_user_filter {
    public function onCreate(): bool {
      $this->impl = new \__SystemLib\ChunkedInflator();
      return true;
    }

    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        if ($this->impl->eof()) {
          return \PSFS_ERR_FATAL;
        }
        $this_chunk = $this->impl->inflateChunk($bucket->data);
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream, $this_chunk)
        );
      }

      return \PSFS_PASS_ON;
    }
  }

  class StringToUpperStreamFilter extends \php_user_filter {
    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream, strtoupper($bucket->data))
        );
      }
      return \PSFS_PASS_ON;
    }
  }

  class StringToLowerStreamFilter extends \php_user_filter {
    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream, strtolower($bucket->data))
        );
      }
      return \PSFS_PASS_ON;
    }
  }

  class StringRot13StreamFilter extends \php_user_filter {
    public function filter($in, $out, &$consumed, $closing): int {
      while ($bucket = stream_bucket_make_writeable($in)) {
        stream_bucket_append(
          $out,
          stream_bucket_new($this->stream, str_rot13($bucket->data))
        );
      }
      return \PSFS_PASS_ON;
    }
  }
}
