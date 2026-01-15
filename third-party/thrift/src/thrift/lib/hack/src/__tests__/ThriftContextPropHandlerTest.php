<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

<<Oncalls('thrift')>>
final class ThriftContextPropHandlerTest extends WWWTest {
  use ClassLevelTest;
  private static function readTFMForTest(string $v): ThriftFrameworkMetadata {
    $transport = Base64::decode($v);
    $buf = new TMemoryBuffer($transport);
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->read($prot);
    return $tfm;
  }

  public function testValue(): void {
    ThriftContextPropState::get()->setRequestId("abcba");
    MCPContext::setGlobal__UNSAFE(MCPOriginIDs::INTEGRITY);
    $v = ThriftContextPropHandler::makeV();
    if ($v !== null) {
      $transport = Base64::decode($v);
      $buf = new TMemoryBuffer($transport);
      $prot = new TCompactProtocolAccelerated($buf);
      $tfm = ThriftFrameworkMetadata::withDefaultValues();
      expect($tfm->request_id)->toBeNull();
      $tfm->read($prot);
      expect($tfm->request_id)->toEqual("abcba");
      expect($tfm->origin_id)->toEqual(MCPOriginIDs::INTEGRITY);
    } else {
      expect(true)->toBeFalse();
    }
  }

  public function testValueWithUniverse(): void {
    ThriftContextPropState::get()->setPrivacyUniverse(1234);
    $v = ThriftContextPropHandler::makeV();
    if ($v is null) {
      expect($v)->toNotBeNull();
      return;
    }

    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->read(
      new TCompactProtocolAccelerated(new TMemoryBuffer(Base64::decode($v))),
    );
    expect($tfm->privacyUniverse)->toEqual(1234);
  }
}
