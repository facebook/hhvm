<?php

function alpha() {
  throw new Error('Foo');
}

function beta() {
  alpha();
}

function gamma() {
  beta();
}

function main() {
  gamma();
}

main();
