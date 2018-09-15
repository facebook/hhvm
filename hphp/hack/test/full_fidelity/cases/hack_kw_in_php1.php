<?php

class inout {}

function foo(): inout {
  return new inout();
}

class bar extends inout {}




class suspend {}

function s(): suspend {
  return new suspend();
}

class S extends suspend {}
