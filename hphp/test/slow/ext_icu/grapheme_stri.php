<?hh

<<__EntryPoint>>
function main_grapheme_stri() :mixed{
var_dump(grapheme_stripos('DEJA', 'e'));
var_dump(grapheme_stripos('DÉJÀ', 'é'));
var_dump(grapheme_stristr('DEJA', 'e'));
var_dump(grapheme_stristr('DÉJÀ', 'é'));
}
