<?hh


<<__EntryPoint>>
function main_normalizer() :mixed{
var_dump(Normalizer::isNormalized("\xC3\x85"));
var_dump(Normalizer::isNormalized("A\xCC\x8A"));
var_dump(Normalizer::normalize("A\xCC\x8A", Normalizer::FORM_C) ===
   "\xC3\x85");
}
