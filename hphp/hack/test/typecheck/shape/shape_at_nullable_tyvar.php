<?hh

function coerceWraps<T>(
    TypeStructure<T> $ts,
    mixed $value,
):T {
  throw new Exception("A");
}

type AdsAPIAdOmnichannelLinkSpecParam = shape(
  ?'a' =>
    ?AdsAPIAdOmnichannelLinkSpecAppParam,
  ...
);

type AdsAPIAdOmnichannelLinkSpecAppParam = shape(
  'b' => string,
  ...
);

function myat<T>(shape('b' => T, ...) $s, arraykey $_):T {
  throw new Exception("A");
}
function expectString(string $_):void { }
function testit(mixed $m):void {
  $omnichannel_link_spec_wrap = coerceWraps(
      type_structure_for_alias(AdsAPIAdOmnichannelLinkSpecParam::class),
      $m,
    );

    $omnichannel_link_spec_app = Shapes::idx(
      $omnichannel_link_spec_wrap,
      'a',
    );

    if ($omnichannel_link_spec_app is null) {
      return;
    }
    $application_id = Shapes::at(
      $omnichannel_link_spec_app,
      'b',
    );
    expectString($application_id);
}
