<?php

class using {}

function foo(): using {
  return new using();
}

class bar extends using {}
