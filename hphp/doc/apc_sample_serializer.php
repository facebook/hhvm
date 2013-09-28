<?php

/**
 * Write .cpp files for building libapc_prime.so that can be loaded by an
 * HPHP-compiled server at startup time.
 */
class CacheArchiveHphpSerializer extends CacheArchiveSerializer {
  public function __construct($archive_name) {
    $this->archive_name = $archive_name;
  }

  public function getName() { return 'hphp';}
  public function unserialize($data) {}

  public function serialize($data) {
    $tmp = 0;
    $out = "\n#include \"apc_prime.h\"\n\n";
    $out .= "namespace HPHP {\n";
    $out .= str_repeat('/', 79)."\n\n";
    $out .= "APC_BEGIN(".$this->getArchiveId().");\n";

    unset($data['dummy_key']);
    $this->serializeStrings($out, $data);
    $this->serializeInt64  ($out, $data);
    $this->serializeObjects($out, $data);
    $this->serializeChars  ($out, $data);
    $this->serializeThrifts($out, $data);
    $this->serializeOthers ($out, $data);

    $out .= "APC_END(".$this->getArchiveId().");\n\n";
    $out .= str_repeat('/', 79)."\n";
    $out .= "}\n";
    return $out;
  }

  private function getArchiveId() {
    return preg_replace('/-/', '_', $this->archive_name);
  }

  private function s($s) {
    static $esc_chars = "\0\n\r\t\\\"?";
    $slen = strlen($s);
    $s = addcslashes($s, $esc_chars);
    return "\"$s\",S($slen)";
  }

  private function serializeChars(&$out, &$data) {
    $i = 0;
    $keys = "static const char *char_keys[] = {\n";
    $values = "static char char_values[] = {\n";
    foreach ($data as $k => $v) {
      if ($v === null || is_bool($v)) {
        $keys .= $this->s($k).',';
        if ($v === null) {
          $values .= '2,';
        } else if ($v) {
          $values .= '1,';
        } else {
          $values .= '0,';
        }
        if (++$i % 10 == 0) {
          $keys .= "\n";
          $values .= "\n";
        }
        unset($data[$k]);
      }
    }
    $keys .= "\nNULL};\n";
    $values .= "\n};\n";

    $out .= $keys;
    $out .= $values;
  }

  private function serializeInt64(&$out, &$data) {
    $i = 0;
    $keys = "static const char *int_keys[] = {\n";
    $values = "static int64 int_values[] = {\n";
    foreach ($data as $k => $v) {
      if (is_int($v)) {
        $keys .= $this->s($k).',';
        if ($v >= (1 << 32)) {
          $values .= $v.'LL,';
        } else {
          $values .= $v.',';
        }
        if (++$i % 10 == 0) {
          $keys .= "\n";
          $values .= "\n";
        }
        unset($data[$k]);
      }
    }
    $keys .= "\nNULL};\n";
    $values .= "\n};\n";

    $out .= $keys;
    $out .= $values;
  }

  private function serializeStrings(&$out, &$data) {
    $i = 0;
    $kvs = "static const char *strings[] = {\n";
    foreach ($data as $k => $v) {
      if (is_string($v)) {
        $kvs .= $this->s($k).','.$this->s($v).',';
        if (++$i % 10 == 0) {
          $kvs .= "\n";
        }
        unset($data[$k]);
      }
    }
    $kvs .= "\nNULL};\n";
    $out .= $kvs;
  }

  private function serializeObjects(&$out, &$data) {
    $i = 0;
    $kvs = "static const char *objects[] = {\n";
    foreach ($data as $k => $v) {
      if (is_object($v)) {
        $kvs .= $this->s($k).','.$this->s(serialize($v)).',';
        if (++$i % 10 == 0) {
          $kvs .= "\n";
        }
        unset($data[$k]);
      }
    }
    $kvs .= "\nNULL};\n";
    $out .= $kvs;
  }

  private function serializeThrifts(&$out, &$data) {
    $i = 0;
    $kvs = "static const char *thrifts[] = {\n";
    foreach ($data as $k => $v) {
      $sv = fb_serialize($v);
      if ($sv) {
        $kvs .= $this->s($k).','.$this->s($sv).',';
        if (++$i % 10 == 0) {
          $kvs .= "\n";
        }
        unset($data[$k]);
      }
    }
    $kvs .= "\nNULL};\n";
    $out .= $kvs;
  }

  private function serializeOthers(&$out, &$data) {
    $i = 0;
    $kvs = "static const char *others[] = {\n";
    foreach ($data as $k => $v) {
      $kvs .= $this->s($k).','.$this->s(serialize($v)).',';
      if (++$i % 10 == 0) {
        $kvs .= "\n";
      }
    }
    $kvs .= "\nNULL};\n";
    $out .= $kvs;
  }
}
