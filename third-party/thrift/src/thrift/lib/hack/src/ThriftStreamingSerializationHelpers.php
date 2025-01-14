<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final abstract class ThriftStreamingSerializationHelpers {

  public static function encodeStreamHelper<
    TStreamPayloadType as IResultThriftStruct with {
      type TResult = TStreamType },
    TStreamType,
  >(
    classname<TStreamPayloadType> $payload_classname,
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
        } else {
          // Undeclared thrift exception: Wrap with TApplicationException
          $result = new \TApplicationException(
            $ex->getMessage()."\n".$ex->getTraceAsString(),
          );
        }
      } else {
        if ($result is ThriftSyncStructWithResult) {
          /* HH_FIXME[4110] Implicit pessimisation */
          $result->success = $payload;
        } else if ($result is ThriftAsyncStructWithResult) {
          /* HH_FIXME[4110] Implicit pessimisation */
          $result->success = $payload;
        }
      }
      $use_accelearted_serialization =
        JustKnobs::eval('thrift/hack:stream_accelerated_serialization');
      if (
        $use_accelearted_serialization &&
        $protocol is \TBinaryProtocolAccelerated
      ) {
        thrift_protocol_write_binary_struct($protocol, $result);
      } else if (
        $use_accelearted_serialization &&
        $protocol is \TCompactProtocolAccelerated
      ) {
        thrift_protocol_write_compact_struct($protocol, $result);
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
    classname<TStreamPayloadType> $payload_classname,
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

        $transport->resetBuffer();
        $transport->write($stream_payload as nonnull);
        if (
          JustKnobs::eval('thrift/hack:stream_accelerated_deserialization') &&
          $protocol is \TBinaryProtocolAccelerated
        ) {
          $result = thrift_protocol_read_binary_struct(
            $protocol,
            HH\class_to_classname($payload_classname),
          );
        } else if (
          JustKnobs::eval('thrift/hack:stream_accelerated_deserialization') &&
          $protocol is \TCompactProtocolAccelerated
        ) {
          $result = thrift_protocol_read_compact_struct(
            $protocol,
            HH\class_to_classname($payload_classname),
          );
        } else {
          $result = $payload_classname::withDefaultValues();
          $result->read($protocol);
        }
        $protocol->readMessageEnd();
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
