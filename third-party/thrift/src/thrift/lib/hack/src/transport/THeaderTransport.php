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

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

/**
 * Header transport. Writes and reads data with
 *
 * THeaderTransport (any protocol)
 * or TFramedTransport format
 * or unframed TBinaryProtocol.
 * (or TBinary over HTTP if someone wants to implement it)
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class THeaderTransport
  extends TFramedTransport
  implements TTransportSupportsHeaders {
  const int TBINARY_PROTOCOL = 0;
  const int TJSON_PROTOCOL = 1;
  const int TCOMPACT_PROTOCOL = 2;

  const int HEADER_MAGIC = 0x0fff;
  const int HTTP_MAGIC = 0x504f5354; // POST
  const int MAX_FRAME_SIZE = 0x3fffffff;

  // Transforms
  const int ZLIB_TRANSFORM = 0x01;
  const int HMAC_TRANSFORM = 0x02;
  const int SNAPPY_TRANSFORM = 0x03;

  // Infos
  const int INFO_KEYVALUE = 0x01;

  // Client types
  const int HEADER_CLIENT_TYPE = 0x00;
  const int FRAMED_DEPRECATED = 0x01;
  const int UNFRAMED_DEPRECATED = 0x02;
  const int HTTP_CLIENT_TYPE = 0x03;
  const int UNKNOWN_CLIENT_TYPE = 0x04;

  // default to binary
  protected int $protoId_ = self::TBINARY_PROTOCOL;
  protected int $clientType_ = self::HEADER_CLIENT_TYPE;
  protected Set<int> $supportedProtocols;
  protected HH_FIXME\WRONG_TYPE<Vector<int>> $readTrans_;
  protected Vector<int> $writeTrans_;
  protected int $seqId_ = 0;
  protected int $flags_ = 0;

  protected Map<string, string> $readHeaders;
  protected Map<string, string> $writeHeaders;
  protected Map<string, string> $persistentWriteHeaders;

  const string IDENTITY_HEADER = "identity";
  const string ID_VERSION_HEADER = "id_version";
  const int ID_VERSION = 1;
  protected ?string $identity = null;

  /**
   * Constructor.
   *
   * @param TTransport $transport Underlying transport
   */
  public function __construct(
    ?TTransport $transport = null,
    ?Traversable<int> $protocols = null,
  )[] {
    parent::__construct($transport, true, true);
    $this->readTrans_ = Vector {};
    $this->writeTrans_ = Vector {};
    $this->readHeaders = Map {};
    $this->writeHeaders = Map {};
    $this->persistentWriteHeaders = Map {};
    $this->supportedProtocols = Set::fromArrays(
      vec[
        self::HEADER_CLIENT_TYPE,
        self::FRAMED_DEPRECATED,
        self::UNFRAMED_DEPRECATED,
        self::HTTP_CLIENT_TYPE,
      ],
      $protocols ? vec($protocols) : vec[],
    );
  }

  public function setProtocolID(int $protoId)[write_props]: this {
    $this->protoId_ = $protoId;
    return $this;
  }

  public function addTransform(int $trans_id)[write_props]: this {
    $this->writeTrans_[] = $trans_id;
    return $this;
  }

  public function resetProtocol()[zoned_shallow]: void {
    if ($this->clientType_ === self::HTTP_CLIENT_TYPE) {
      $this->flush();
    }
    $this->clientType_ = self::HEADER_CLIENT_TYPE;
    $this->readFrame(0);
  }

  <<__Deprecated("Use `setWriteHeader` instead")>>
  public function setHeader(
    string $str_key,
    string $str_value,
  )[write_props]: this {
    return $this->setWriteHeader($str_key, $str_value);
  }

  public function setWriteHeader(
    string $str_key,
    string $str_value,
  )[write_props]: this {
    $this->writeHeaders[$str_key] = (string)$str_value;
    return $this;
  }

  public function setPersistentHeader(
    string $str_key,
    string $str_value,
  )[write_props]: this {
    $this->persistentWriteHeaders[$str_key] = (string)$str_value;
    return $this;
  }

  public function getWriteHeaders()[]: KeyedContainer<string, string> {
    return $this->writeHeaders;
  }

  public function getPersistentWriteHeaders(
  )[]: KeyedContainer<string, string> {
    return $this->persistentWriteHeaders;
  }

  <<__Deprecated("Use `getReadHeaders` instead")>>
  public function getHeaders()[]: KeyedContainer<string, string> {
    return $this->getReadHeaders();
  }

  public function getReadHeaders()[]: KeyedContainer<string, string> {
    return $this->readHeaders;
  }

  <<__Deprecated("Use `clearWriteHeaders` instead")>>
  public function clearHeaders()[write_props]: void {
    $this->clearWriteHeaders();
  }

  public function clearWriteHeaders()[write_props]: void {
    $this->writeHeaders = Map {};
  }

  public function clearPersistentHeaders()[write_props]: void {
    $this->persistentWriteHeaders = Map {};
  }

  public function getPeerIdentity()[write_props]: ?string {
    if ($this->readHeaders->contains(self::IDENTITY_HEADER)) {
      if (
        (int)$this->readHeaders[self::ID_VERSION_HEADER] === self::ID_VERSION
      ) {
        return $this->readHeaders[self::IDENTITY_HEADER];
      }
    }

    return null;
  }

  public function setIdentity(string $identity)[write_props]: this {
    $this->identity = $identity;
    return $this;
  }

  /**
   * Reads from the buffer. When more data is required reads another entire
   * chunk and serve future reads out of that.
   *
   * @param int $len How much data
   */
  <<__Override>>
  public function read(int $len)[zoned_shallow]: string {
    if ($this->clientType_ === self::UNFRAMED_DEPRECATED) {
      return $this->transport_->readAll($len);
    }

    if (Str\length($this->rBuf_) === 0) {
      $this->readFrame($len);
    }

    // Return substr
    $out = PHP\substr($this->rBuf_, $this->rIndex_, $len);
    $this->rIndex_ += $len;

    if (Str\length($this->rBuf_) <= $this->rIndex_) {
      $this->rBuf_ = '';
      $this->rIndex_ = 0;
    }
    return $out;
  }

  /**
   * Reads a chunk of data into the internal read buffer.
   */
  private function readFrame(int $req_sz)[zoned_shallow]: void {
    $buf = $this->transport_->readAll(4);
    $val = PHP\unpack('N', $buf);
    $sz = $val[1];

    if (($sz & TBinaryProtocol::VERSION_MASK) === TBinaryProtocol::VERSION_1) {
      $this->clientType_ = self::UNFRAMED_DEPRECATED;
      if ($req_sz <= 4) {
        $this->rBuf_ = $buf;
      } else {
        $this->rBuf_ = $buf.$this->transport_->readAll($req_sz - 4);
      }
    } else if ($sz === self::HTTP_MAGIC) {
      throw new TTransportException(
        'PHP HeaderTransport does not support HTTP',
        TTransportException::INVALID_CLIENT,
      );
    } else {
      // Either header format or framed. Check next byte
      $buf2 = $this->transport_->readAll(4);
      $val2 = PHP\unpack('N', $buf2);
      $version = $val2[1];
      if (
        ($version & TBinaryProtocol::VERSION_MASK) ===
          TBinaryProtocol::VERSION_1
      ) {
        $this->clientType_ = self::FRAMED_DEPRECATED;
        if ($sz - 4 > self::MAX_FRAME_SIZE) {
          throw new TTransportException(
            'Frame size is too large',
            TTransportException::INVALID_FRAME_SIZE,
          );
        }
        $this->rBuf_ = $buf2.$this->transport_->readAll($sz - 4);
      } else if (($version & 0xffff0000) === (self::HEADER_MAGIC << 16)) {
        $this->clientType_ = self::HEADER_CLIENT_TYPE;
        if ($sz - 4 > self::MAX_FRAME_SIZE) {
          throw new TTransportException(
            'Frame size is too large',
            TTransportException::INVALID_FRAME_SIZE,
          );
        }
        $this->flags_ = ($version & 0x0000ffff);
        // read seqId
        $buf3 = $this->transport_->readAll(4);
        $val3 = PHP\unpack('N', $buf3);
        $this->seqId_ = $val3[1];
        // read header_size
        $buf4 = $this->transport_->readAll(2);
        $val4 = PHP\unpack('n', $buf4);
        $header_size = $val4[1];

        $data = $buf.$buf2.$buf3.$buf4;
        $index = Str\length($data);

        $data .= $this->transport_->readAll($sz - 10);
        $this->readHeaderFormat($sz - 10, $header_size, $data, $index);
      } else {
        $this->clientType_ = self::UNKNOWN_CLIENT_TYPE;
        throw new TTransportException(
          'Unknown client type',
          TTransportException::INVALID_CLIENT,
        );
      }
    }

    if (!$this->supportedProtocols->contains($this->clientType_)) {
      throw new TTransportException(
        'Client type not supported on this server',
        TTransportException::INVALID_CLIENT,
      );
    }

  }

  protected function readVarint(string $data, inout int $index)[]: int {
    $result = 0;
    $shift = 0;
    while (true) {
      $x = PHP\substr($data, $index, 1);
      $index++;
      $byte = PHP\ord($x);
      $result |= ($byte & 0x7f) << $shift;
      if (($byte >> 7) === 0) {
        return $result;
      }
      $shift += 7;
    }

    throw new TTransportException("THeaderTransport: You shouldn't be here");
  }

  protected function getVarint(int $data)[]: string {
    $out = "";
    while (true) {
      if (($data & ~0x7f) === 0) {
        $out .= PHP\chr($data);
        break;
      } else {
        $out .= PHP\chr(($data & 0xff) | 0x80);
        $data = $data >> 7;
      }
    }
    return $out;
  }

  protected function writeString(string $str)[]: string {
    $buf = $this->getVarint(Str\length($str));
    $buf .= $str;
    return $buf;
  }

  protected function readString(
    string $data,
    inout int $index,
    int $limit,
  )[]: string {
    $str_sz = $this->readVarint($data, inout $index);
    if ($str_sz + $index > $limit) {
      throw new TTransportException(
        'String read too long',
        TTransportException::INVALID_FRAME_SIZE,
      );
    }

    $str = PHP\substr($data, $index, $str_sz);
    $index += $str_sz;
    return $str;
  }

  protected function readHeaderFormat(
    int $sz,
    int $header_size,
    string $data,
    int $index,
  )[write_props]: void {
    $this->readTrans_ = Vector {};

    $header_size = $header_size * 4;
    if ($header_size > $sz) {
      throw new TTransportException(
        'Header size is larger than frame',
        TTransportException::INVALID_FRAME_SIZE,
      );
    }
    $end_of_header = $index + $header_size;

    $this->protoId_ = $this->readVarint($data, inout $index);
    $numHeaders = $this->readVarint($data, inout $index);
    if (
      $this->protoId_ === 1 && $this->clientType_ !== self::HTTP_CLIENT_TYPE
    ) {
      throw new TTransportException(
        'Trying to recv JSON encoding over binary',
        TTransportException::INVALID_CLIENT,
      );
    }

    // Read in the headers.  Data for each header varies.
    for ($i = 0; $i < $numHeaders; $i++) {
      $transId = $this->readVarint($data, inout $index);
      switch ($transId) {
        case self::ZLIB_TRANSFORM:
        case self::SNAPPY_TRANSFORM:
          $this->readTrans_[] = $transId;
          break;

        case self::HMAC_TRANSFORM:
          throw TApplicationException::fromShape(shape(
            'message' => 'Hmac transform no longer supported',
            'code' => TApplicationException::INVALID_TRANSFORM,
          ));

        default:
          throw TApplicationException::fromShape(shape(
            'message' => 'Unknown transform in client request',
            'code' => TApplicationException::INVALID_TRANSFORM,
          ));
      }
    }
    // Make sure that the read transforms are applied in the reverse order
    // from when the data was written.
    $this->readTrans_ = HH\FIXME\UNSAFE_CAST<dict<arraykey, int>, Vector<int>>(
      PHP\array_reverse($this->readTrans_),
      'Exposed by typing PHP\array_reduce',
    );

    // Read the info headers
    $this->readHeaders = Map {};
    while ($index < $end_of_header) {
      $infoId = $this->readVarint($data, inout $index);
      switch ($infoId) {
        case self::INFO_KEYVALUE:
          $num_keys = $this->readVarint($data, inout $index);
          for ($i = 0; $i < $num_keys; $i++) {
            $strKey = $this->readString($data, inout $index, $end_of_header);
            $strValue = $this->readString($data, inout $index, $end_of_header);
            $this->readHeaders[$strKey] = $strValue;
          }
          break;
        default:
          // End of infos
          break;
      }
    }

    $this->rBuf_ =
      $this->untransform(PHP\substr($data, $end_of_header, $sz - $header_size));
  }

  protected function transform(string $data)[]: string {
    // the only way we know that these possibly iterated compressors
    // will produce reeasonable results is by complicated promises of the way
    // thrift sets things up, which Hack has no way to understand.
    foreach ($this->writeTrans_ as $trans) {
      switch ($trans) {
        case self::ZLIB_TRANSFORM:
          $data = HH\FIXME\UNSAFE_CAST<dynamic, string>(PHP\gzcompress($data));
          break;

        case self::SNAPPY_TRANSFORM:
          $data = HH\FIXME\UNSAFE_CAST<dynamic, string>(PHP\sncompress($data));
          break;

        default:
          throw new TTransportException(
            'Unknown transform during send',
            TTransportException::INVALID_TRANSFORM,
          );
      }
    }
    return ($data);
  }

  protected function untransform(string $data)[]: string {
    // the only way we know that these possibly iterated compressors
    // will produce reeasonable results is by complicated promises of the way
    // thrift sets things up, which Hack has no way to understand.
    foreach ($this->readTrans_ as $trans) {
      switch ($trans) {
        case self::ZLIB_TRANSFORM:
          $data =
            HH\FIXME\UNSAFE_CAST<dynamic, string>(PHP\gzuncompress($data));
          break;

        case self::SNAPPY_TRANSFORM:
          $data =
            HH\FIXME\UNSAFE_CAST<dynamic, string>(PHP\snuncompress($data));
          break;

        default:
          throw TApplicationException::fromShape(shape(
            'message' => 'Unknown transform during recv',
            'code' => TTransportException::INVALID_TRANSFORM,
          ));
      }
    }
    return ($data);
  }

  /**
   * Writes the output buffer in header format, or format
   * client responded with (framed, unframed, http)
   */
  <<__Override>>
  public function flush()[zoned_shallow]: void {
    $this->flushImpl(false);
  }

  <<__Override>>
  public function onewayFlush()[zoned_shallow]: void {
    $this->flushImpl(true);
  }

  private function flushImpl(bool $oneway)[zoned_shallow]: void {
    if (Str\length($this->wBuf_) === 0) {
      if ($oneway) {
        $this->transport_->onewayFlush();
      } else {
        $this->transport_->flush();
      }
      return;
    }

    $out = $this->transform($this->wBuf_);

    // Note that we clear the internal wBuf_ prior to the underlying write
    // to ensure we're in a sane state (i.e. internal buffer cleaned)
    // if the underlying write throws up an exception
    $this->wBuf_ = '';

    if (
      $this->protoId_ === 1 && $this->clientType_ !== self::HTTP_CLIENT_TYPE
    ) {
      throw new TTransportException(
        'Trying to send JSON encoding over binary',
        TTransportException::INVALID_CLIENT,
      );
    }

    if ($this->clientType_ === self::HEADER_CLIENT_TYPE) {
      $transformData = '';

      $num_headers = 0;
      // For now, no transform requires data.
      foreach ($this->writeTrans_ as $trans) {
        ++$num_headers;
        $transformData .= $this->getVarint($trans);
      }

      // Add in special flags.
      if ($this->identity !== null) {
        $this->writeHeaders[self::ID_VERSION_HEADER] = (string)self::ID_VERSION;
        $this->writeHeaders[self::IDENTITY_HEADER] = $this->identity;
      }

      $infoData = '';
      if ($this->writeHeaders || $this->persistentWriteHeaders) {
        $infoData .= $this->getVarint(self::INFO_KEYVALUE);
        $infoData .= $this->getVarint(
          C\count($this->writeHeaders) + C\count($this->persistentWriteHeaders),
        );
        foreach ($this->persistentWriteHeaders as $str_key => $str_value) {
          $infoData .= $this->writeString($str_key);
          $infoData .= $this->writeString($str_value);
        }
        foreach ($this->writeHeaders as $str_key => $str_value) {
          $infoData .= $this->writeString($str_key);
          $infoData .= $this->writeString($str_value);
        }
      }
      $this->writeHeaders = Map {};

      $headerData =
        $this->getVarint($this->protoId_).$this->getVarint($num_headers);
      $header_size = Str\length($transformData) +
        Str\length($infoData) +
        Str\length($headerData);
      $paddingSize = 4 - ($header_size % 4);
      $header_size += $paddingSize;

      $buf = (string)PHP\pack('nn', self::HEADER_MAGIC, $this->flags_);
      $buf .= (string)PHP\pack('Nn', $this->seqId_, $header_size / 4);

      $buf .= $headerData.$transformData;
      $buf .= $infoData;

      // Pad out the header with 0x00
      for ($i = 0; $i < $paddingSize; $i++) {
        $buf .= (string)PHP\pack('C', '\0');
      }

      // Append the data
      $buf .= $out;

      // Prepend the size.
      $buf = (string)PHP\pack('N', Str\length($buf)).$buf;
    } else if ($this->clientType_ === self::FRAMED_DEPRECATED) {
      $buf = (string)PHP\pack('N', Str\length($out));
      $buf .= $out;
    } else if ($this->clientType_ === self::UNFRAMED_DEPRECATED) {
      $buf = $out;
    } else if ($this->clientType_ === self::HTTP_CLIENT_TYPE) {
      throw new TTransportException(
        'HTTP not implemented',
        TTransportException::INVALID_CLIENT,
      );
    } else {
      throw new TTransportException(
        'Unknown client type',
        TTransportException::INVALID_CLIENT,
      );
    }

    if (Str\length($buf) > self::MAX_FRAME_SIZE) {
      throw new TTransportException(
        'Attempting to send oversize frame',
        TTransportException::INVALID_FRAME_SIZE,
      );
    }

    $this->transport_->write($buf);
    if ($oneway) {
      $this->transport_->onewayFlush();
    } else {
      $this->transport_->flush();
    }
  }
}
