<?php

namespace lang {

interface Value {}

}

namespace {

use lang\Value;

$x = new class() implements Value {
};

\var_dump($x);

}
