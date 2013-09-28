<?php

namespace A;
class Cl { public function __construct() { return "a"; } }

namespace B;
class Cl { public function __construct() { return "b"; } }
use A\Cl;
