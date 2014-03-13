<?php

namespace HH {

interface KeyedIterable extends \HH\Iterable, \HH\KeyedTraversable {
  public function mapWithKey($callback);
  public function filterWithKey($callback);
  public function keys();
}

}
