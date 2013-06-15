<?php

var_dump(Normalizer::isnormalized("\xC3\x85"));
var_dump(Normalizer::isnormalized("A\xCC\x8A"));
var_dump(Normalizer::normalize("A\xCC\x8A", Normalizer::FORM_C) ===
   "\xC3\x85");
