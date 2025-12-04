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

final abstract class ThriftStreamingSerializationHelpers {

  public static function encodeStreamHelper<
    TStreamPayloadType as IResultThriftStruct with {
      type TResult = TStreamType },
    TStreamType,
  >(
    class<TStreamPayloadType> $payload_classname,
    TProtocol $protocol,
  ): (function(?TStreamType, ?Exception): (string, bool)) {
    return (?TStreamType $payload, ?\Exception $ex) ==> {
      $transport = $protocol->getTransport();
      invariant(
        $transport is \TMemoryBuffer,
        "Stream/Sink methods require TMemoryBuffer transport",
      );

      $result = $payload_classname::withDefaultValues();
      $is_application_ex = false;

      if ($ex !== null && !($ex is \TException && $result->setException($ex))) {
        $is_application_ex = true;
        if ($ex is \TApplicationException) {
          $result = $ex;
        } else if ($ex is \ThriftApplicationException) {
          $result = new TApplicationException(
            $ex->getMessage(),
            TApplicationException::UNKNOWN,
          );
          $result->setPrevious($ex);
        } else {
          // Undeclared thrift exception: Wrap with TApplicationException
          $result = new \TApplicationException(
            $ex->getMessage()."\n".$ex->getTraceAsString(),
          );
          $result->setPrevious($ex);
        }

      } else {
        if ($result is ThriftSyncStructWithResult) {
          $result->success = HH\FIXME\UNSAFE_CAST<?TStreamType, ?nothing>(
            $payload,
            'FIXME[4110] Implicit pessimisation',
          );
        } else if ($result is ThriftAsyncStructWithResult) {
          $result->success = HH\FIXME\UNSAFE_CAST<?TStreamType, ?nothing>(
            $payload,
            'FIXME[4110] Implicit pessimisation',
          );
        }
      }
      if ($protocol is \TBinaryProtocolAccelerated) {
        thrift_protocol_write_binary_struct($protocol, $result);
      } else if ($protocol is \TCompactProtocolAccelerated) {
        if (TCompactSerializer::useCompactStruct()) {
          return tuple(
            thrift_protocol_write_compact_struct_to_string(
              $result,
              TCompactProtocolBase::VERSION,
            ),
            $is_application_ex,
          );
        } else {
          thrift_protocol_write_compact_struct($protocol, $result);
        }
      } else {
        $result->write($protocol);
        $transport->flush();
      }
      $msg = $transport->getBuffer();
      $transport->resetBuffer();
      return tuple($msg, $is_application_ex);
    };
  }

  public static function decodeStreamHelper<
    TStreamPayloadType as IResultThriftStruct with {
      type TResult = TStreamType },
    TStreamType,
  >(
    class<TStreamPayloadType> $payload_classname,
    string $name,
    TProtocol $protocol,
    shape(?'read_options' => int) $_options = shape(),
  ): (function(?string, ?\Exception): TStreamType) {
    return (?string $stream_payload, ?\Exception $ex) ==> {
      try {
        if ($ex is \ThriftApplicationException) {
          $ex = new TApplicationException(
            $ex->getMessage(),
            TApplicationException::UNKNOWN,
          );
        }
        if ($ex !== null) {
          throw $ex;
        }
        $transport = $protocol->getTransport();
        invariant(
          $transport is \TMemoryBuffer,
          "Stream/Sink methods require TMemoryBuffer transport",
        );
        if (
          $protocol is \TCompactProtocolAccelerated &&
          TCompactSerializer::useCompactStruct()
        ) {
          $result = thrift_protocol_read_compact_struct_from_string(
            $stream_payload as nonnull,
            HH\class_to_classname($payload_classname),
            0, //options
            TCompactProtocolBase::VERSION,
          );
        } else {
          $transport->resetBuffer();
          $transport->write($stream_payload as nonnull);
          if ($protocol is \TBinaryProtocolAccelerated) {
            $result = thrift_protocol_read_binary_struct(
              $protocol,
              HH\class_to_classname($payload_classname),
            );
          } else if ($protocol is \TCompactProtocolAccelerated) {
            $result = thrift_protocol_read_compact_struct(
              $protocol,
              HH\class_to_classname($payload_classname),
            );
          } else {
            $result = $payload_classname::withDefaultValues();
            $result->read($protocol);
          }
          $protocol->readMessageEnd();
        }
      } catch (\THandlerShortCircuitException $ex) {
        throw $ex->result;
      }
      if ($result is ThriftSyncStructWithResult) {
        $successful_result = $result->success;
      } else if ($result is ThriftAsyncStructWithResult) {
        $successful_result = $result->success;
      } else {
        $successful_result = null;
      }
      if ($successful_result !== null) {
        return HH\FIXME\UNSAFE_CAST<mixed, TStreamType>(
          $successful_result,
          'FIXME[4110] Type error uncovered by True Types, potential incompleteness (see https://fburl.com/workplace/ngmpvd6l)',
        );
      }

      $exception = $result->checkForException();
      if ($exception is nonnull) {
        throw $exception;
      }

      throw new \TApplicationException(
        $name." failed: unknown result",
        TApplicationException::MISSING_RESULT,
      );
    };
  }

}
