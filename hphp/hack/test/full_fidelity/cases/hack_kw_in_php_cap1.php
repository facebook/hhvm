<?php

class InOut {}

function foo(): InOut {
  return new InOut();
}

class bar extends InOut {}
