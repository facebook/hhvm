<?php

function salut($hello) {
  var_dump(hh\asm('
    CGetL $hello
    JmpZ goodbye
    String "Hello\n"
    Jmp done
goodbye:
    String "Goodbye\n"
done:
    Print
    PopC
    Int 42
  '));
}

salut(true);
salut(false);
