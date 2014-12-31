<?php

class APCIterator implements Iterator{

  private $initialized = false;
  private $constructed = false;
  private $index = -1; // Gets increased when inited
  private $totals = null;
  private $format;
  private $search;
  private $info;


  // Only formats available: type, key, value, memsize, ttl
  // throws notice on different flags
  //
  // $chunk_size and $list doesn't do anything
  // I don't feel the need of chunks, since I don't save entries anyway.
  // I'm not quite sure what $list does, however
  // I believe it isn't implemented yet
  public function __construct($cache, $search = null, $format = APC_ITER_ALL,
                              $chunk_size = 100, $list = APC_LIST_ACTIVE) {

    if(!function_exists('apc_add')) {
      trigger_error("APC must be enabled to use APCIterator", E_ERROR);
    }

    if ($chunk_size < 0) {
      trigger_error("APCIterator chunk size must be 0 or greater.",
                    E_ERROR);
    }
    if ($format > APC_ITER_ALL) {
      trigger_error("APCIterator format is invalid.", E_ERROR);
    }

    if ($format & (
      APC_ITER_FILENAME |
      APC_ITER_DEVICE |
      APC_ITER_INODE |
      APC_ITER_MD5 |
      APC_ITER_NUM_HITS |
      APC_ITER_MTIME |
      APC_ITER_CTIME |
      APC_ITER_DTIME |
      APC_ITER_ATIME |
      APC_ITER_REFCOUNT
    )) {
      trigger_error(
        "Format values FILENAME, DEVICE, INODE, MD5, NUM_HITS, MTIME,".
        " CTIME, DTIME, ATIME, REFCOUNT not supported yet.",
        E_NOTICE
      );
    }

    if ($list != APC_LIST_ACTIVE && $list != APC_LIST_DELETED) {
        trigger_error("APCIterator invalid list type.", E_WARNING);
        return;
    }
    if (strcasecmp($cache, 'user') === 0) {
      // Only user implemented
    } else {
      throw new Exception(
        "Filehits cache not implemented."
      );
    }

    $this->search = $search;
    $this->format = $format;
    $this->initialized = false;  // will be initialized upon first access
    $this->constructed = true;
  }


  public function valid() {
    if (!$this->constructed) {
      return false;
    }
    return count($this->getInfo()) > $this->index;
  }

  public function next() {
    if (!$this->valid()) {
      return false;
    }
    ++$this->index;
    if ($this->search !== null) {
      if (is_array($this->search)) {
        while ($this->valid() &&
               !$this->preg_match_recursive($this->search, $this->key())) {
          ++$this->index;
        }
      } else {
        while ($this->valid() && !preg_match($this->search, $this->key())) {
          ++$this->index;
        }
      }
    }
    return true;
  }

  public function rewind() {
    if (!$this->constructed) {
      return false;
    }
    $this->init();
  }

  public function key() {
    if (!$this->valid()) {
      return false;
    }
    return $this->getInfo()[$this->index]['entry_name'];
  }

  public function current() {
    if (!$this->valid()) return false;
    $info = $this->getInfo()[$this->index];
    $ret = array();
    if ($this->format & APC_ITER_TYPE) {
      $ret['type'] = ($info['type'] == 0) ? 'user' : 'file';
    }
    if ($this->format & APC_ITER_KEY) {
      $ret['key'] = $info['entry_name'];
    }
    if ($this->format & APC_ITER_VALUE) {
      $ret['value'] = apc_fetch($info['entry_name']);
    }
    if ($this->format & APC_ITER_MEM_SIZE) {
      $ret['mem_size'] = $info['mem_size'];
    }
    if ($this->format & APC_ITER_TTL) {
      $ret['ttl'] = $info['ttl'];
    }
    return $ret;
  }

  public function getTotalCount() {
    if (!$this->constructed) {
      return false;
    }
    if (!$this->initialized) {
      $this->init();
    }
    if (!$this->totals) {
      $this->getTotals();
    }
    return $this->totals['count'];
  }

  public function getTotalHits() {
    if (!$this->constructed) {
      return false;
    }
    if (!$this->initialized) {
      $this->init();
    }
    if (!$this->totals) {
      $this->getTotals();
    }
    return $this->totals['hits'];
  }

  public function getTotalSize() {
    if (!$this->constructed) {
      return false;
    }
    if (!$this->initialized) {
      $this->init();
    }
    if (!$this->totals) {
      $this->getTotals();
    }
    return $this->totals['size'];
  }

  private function preg_match_recursive(array $patterns, $string) {
    foreach ($patterns as $pattern) {
      if (preg_match($pattern, $string)) {
        return true;
      }
    }
    return false;
  }

  private function getTotals() {
    $info = $this->getInfo();
    foreach ($info as $list) {
      if ($this->search !== null) {
        if (is_array($this->search)) {
          while (!$this->preg_match_recursive($this->search,
                                              $list['entry_name'])) {
            continue;
          }
        } else {
          if (!preg_match($this->search, $list['entry_name'])) {
            continue;
          }
        }
      }
      $this->totals['size'] += $list['mem_size'];
      $this->totals['hits'] += $list['num_hits'];
    }
    $this->totals['count'] = count($info);
  }

  private function init() {
    $this->info = apc_cache_info()['cache_list'];
    // Order defined by ksort
    ksort($this->info);
    $this->initialized = true;
    $this->index = -1;
    $this->next();
  }

  private function getInfo() {
    if (!$this->initialized) {
      $this->init();
    }
    return $this->info;
  }

  // Used for apc_delete(APCIterator);
  private function delete() {
    if (!$this->constructed) {
      return false;
    }
    if (!$this->initialized) {
      $this->init();
    }
    foreach ($this->info as $key) {
      if ($this->search !== null) {
        if (is_array($this->search)) {
          while (!$this->preg_match_recursive($this->search,
                                              $key['entry_name'])) {
            continue;
          }
        } else {
          if (!preg_match($this->search, $key['entry_name'])) {
            continue;
          }
        }
      }
      apc_delete($key['entry_name']);
    }
    return true;
  }

}
