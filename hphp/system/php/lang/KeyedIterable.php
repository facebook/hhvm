<?php

interface KeyedIterable extends Iterable, KeyedTraversable {
  public function mapWithKey($callback);
  public function filterWithKey($callback);
  public function keys();
  public function kvzip();
}
