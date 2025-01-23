<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

abstract final class ThriftFrameworkMetadataUtils {

  const string EXPERIMENT_IDS_IN_INCOMING_RESPONSE_HEADER_KEY =
    "experiment_ids_in_incoming_response";

  public static function decodeAndJoinExperimentIds(
    KeyedContainer<string, ?string> $propagated_response_headers,
  ): ?string {
    $tfmr_header_key =
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey;
    $tfmr_header = idx($propagated_response_headers, $tfmr_header_key);
    if ($tfmr_header is null) {
      return null;
    }

    $tfmr = ThriftFrameworkMetadataUtils::decodeFrameworkMetadataOnResponse(
      $tfmr_header,
    );
    if ($tfmr->experiment_ids is null) {
      return null;
    }

    $experiment_ids = $tfmr->experiment_ids->get_merge() ?? vec[];
    return Str\join($experiment_ids, ',');
  }

  <<__Memoize>>
  public static function decodeFrameworkMetadataOnResponse(
    string $encoded_response_tfm,
  ): ThriftFrameworkMetadataOnResponse {
    $tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $tfmr->read(
      new TCompactProtocolAccelerated(
        new TMemoryBuffer(Base64::decode($encoded_response_tfm)),
      ),
    );
    return $tfmr;
  }

  <<__Memoize(#KeyedByIC)>>
  public static function encodeThriftFrameworkMetadata(
    ThriftFrameworkMetadata $tfm,
  )[globals, zoned_shallow]: string {
    $buf = new TMemoryBuffer();
    $proto = new TCompactProtocolAccelerated($buf);
    $tfm->write($proto);
    return Base64::encode($buf->getBuffer());
  }

}
