<?hh <<__EntryPoint>> function main(): void {
try { $layer = imagelayereffect('invalid_resource', IMG_EFFECT_REPLACE); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
