<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT = HH\EnumClass\Label<EC, mixed> | string | resource;

enum class EC : mixed {}

function test(CT $ct): void {
    if ($ct is string) {
      hh_expect<string>($ct);
    } else if ($ct is resource) {
        hh_expect<resource>($ct);
    } else {
        hh_expect<HH\EnumClass\Label<EC, mixed>>($ct);
    }
}
