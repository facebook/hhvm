<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftUniversalNameTest extends WWWTest {
  public async function testUniversalHashSha256(): Awaitable<void> {
    $expected = non_crypto_sha256("fbthrift://foo.com/my/type", true);

    expect($expected)->toEqual(
      ThriftUniversalName::getUriHash("foo.com/my/type"),
    );
    expect($expected)->toNotEqual(
      ThriftUniversalName::getUriHash("facebook.com/thrift/Object"),
    );

    // Ensure values haven't changed unintentionally
    expect(PHP\bin2hex($expected))->toEqual(
      '096174249cefadb5ea0d453bcb33ad547601fbfec4b2d795924eeb67d45be646',
    );
  }

  <<DataProvider('makeDataForTestMatchesUniversalHash')>>
  public async function testMatchesUniversalHash(
    string $universal_hash,
    string $hash_prefix,
    bool $should_match,
  ): Awaitable<void> {
    expect(
      ThriftUniversalName::matchesUniversalHash($universal_hash, $hash_prefix),
    )->toEqual($should_match);
  }

  // Data providers

  public static function makeDataForTestMatchesUniversalHash(): dict<
    string,
    shape(
      'universal_hash' => string,
      'hash_prefix' => string,
      'should_match' => bool,
    ),
  > {
    return dict[
      'empty prefix' => shape(
        'universal_hash' => "0123456789ABCDEF0123456789ABCDEF",
        'hash_prefix' => "",
        'should_match' => false,
      ),
      'invalid prefix' => shape(
        'universal_hash' => "0123456789ABCDEF0123456789ABCDEF",
        'hash_prefix' => "1",
        'should_match' => false,
      ),
      'valid prefix' => shape(
        'universal_hash' => "0123456789ABCDEF0123456789ABCDEF",
        'hash_prefix' => "0",
        'should_match' => true,
      ),
      'valid long prefix' => shape(
        'universal_hash' => "0123456789ABCDEF0123456789ABCDEF",
        'hash_prefix' => "0123456789ABCDEF",
        'should_match' => true,
      ),
      'exact match' => shape(
        'universal_hash' => "0123456789ABCDEF0123456789ABCDEF",
        'hash_prefix' => "0123456789ABCDEF0123456789ABCDEF",
        'should_match' => true,
      ),
      'prefix longer than hash' => shape(
        'universal_hash' => "0123456789ABCDEF0123456789ABCDEF",
        'hash_prefix' => "0123456789ABCDEF0123456789ABCDEF0",
        'should_match' => false,
      ),
    ];
  }
}
