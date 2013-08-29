<?php

interface KeyedIterable extends Iterable, KeyedTraversable {
  public function keys();
  public function kvzip();
}
