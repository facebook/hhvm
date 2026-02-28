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
final class TFileStreamTest extends WWWTest {

  use ClassLevelTest;

  public async function testRead(): Awaitable<void> {
    using IntegrationTestDangerousFunctionGuard::forFopen();

    $contents = 'abcdefghijklmnopqrstuvwxyz';
    $file = TempFile::newFile();
    Filesystem::writeFile($file, $contents);

    $stream = new TFileStream(TFileStreamMode::MODE_R, $file->getFileName());
    $stream->setBufferSize(4);
    $stream->open();

    expect($stream->read(3))->toEqual('abc');
    expect($this->getBufferContents($stream))->toEqual('defg');
    expect($stream->peek(1))->toEqual('d');
    expect($stream->peek(2))->toEqual('de');
    expect($stream->peek(2, 1))->toEqual('ef');

    expect($stream->read(3))->toEqual('def');
    expect($this->getBufferContents($stream))->toEqual('g');
    expect($stream->read(1))->toEqual('g');
    expect($this->getBufferContents($stream))->toEqual('');
    expect($stream->read(1))->toEqual('h');
    expect($this->getBufferContents($stream))->toEqual('ijkl');
    expect($stream->peek(4))->toEqual('ijkl');
    expect($stream->peek(6))->toEqual('ijklmn');
    expect($stream->peek(6, 1))->toEqual('jklmno');
    expect($this->getBufferContents($stream))->toEqual('ijklmnopqr');

    expect($stream->read(1))->toEqual('i');
    expect($this->getBufferContents($stream))->toEqual('jklmnopqr');
    expect($stream->read(6))->toEqual('jklmno');
    expect($this->getBufferContents($stream))->toEqual('pqr');
    expect($stream->read(6))->toEqual('pqrstu');
    // Read 6, get 5
    expect($stream->peek(6))->toEqual('vwxyz');
    expect($stream->read(6))->toEqual('vwxyz');

    $stream->close();
  }

  public async function testWrite(): Awaitable<void> {
    using IntegrationTestDangerousFunctionGuard::forFopen();
    $file = TempFile::newFile();

    $stream = new TFileStream(TFileStreamMode::MODE_W, $file->getFileName());
    $stream->setBufferSize(4);
    $stream->open();

    $stream->write('abc');
    expect($this->getBufferContents($stream))->toEqual('abc');
    $stream->write('def');
    expect($this->getBufferContents($stream))->toEqual('');
    $stream->write('g');
    expect($this->getBufferContents($stream))->toEqual('g');
    $stream->write('h');
    expect($this->getBufferContents($stream))->toEqual('gh');
    $stream->write('i');
    expect($this->getBufferContents($stream))->toEqual('ghi');
    $stream->write('jklmno');
    expect($this->getBufferContents($stream))->toEqual('');
    $stream->write('p');
    expect($this->getBufferContents($stream))->toEqual('p');
    $stream->close();
    expect($this->getBufferContents($stream))->toEqual('');

    $contents = Filesystem::readFile($file);
    expect($contents)->toEqual('abcdefghijklmnop');
  }

  private function getBufferContents(TFileStream $stream): string {
    $buffer = BypassVisibility::getInstancePropertyWithTypeHint<
      TFileStreamBuffer,
    >($stream, 'buffer');
    return $buffer->peek(null);
  }
}
