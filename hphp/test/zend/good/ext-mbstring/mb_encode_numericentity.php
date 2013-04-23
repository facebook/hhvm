<?php
$str1 = '¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ';
$str2 = 'ƒΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρςστυφχψωϑϒϖ•…′″‾⁄℘ℑℜ™ℵ←↑→↓↔↵⇐⇑⇒⇓⇔∀∂∃∅∇∈∉∋∏∑−∗√∝∞∠∧∨∩∪∫∴∼≅≈≠≡≤≥⊂⊃⊄⊆⊇⊕⊗⊥⋅⌈⌉⌊⌋〈〉◊♠♣♥♦';
$convmap = array(0x0, 0x2FFFF, 0, 0xFFFF);
echo mb_encode_numericentity($str1, $convmap, "UTF-8")."\n";
echo mb_encode_numericentity($str2, $convmap, "UTF-8")."\n";

$convmap = array(0xFF, 0x2FFFF, 0, 0xFFFF);
echo mb_encode_numericentity('aŒbœcŠdše€fg', $convmap, "UTF-8")."\n";
?>