<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class Frob {
  const FROBS_PER_BLOB = 24;
}

class Widget {
  private $count = 5 * Frob::FROBS_PER_BLOB;
}

var_dump(new Widget);
