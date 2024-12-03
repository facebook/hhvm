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

<<Oncalls('thrift')>> // @oss-disable
abstract class ThriftAsyncProcessor
  extends ThriftProcessorBase
  implements IThriftAsyncProcessor {

  abstract const type TThriftIf as IThriftAsyncIf;

  use TWithMasBuenopathLastSet;

  <<StringMetadataExtractor('Thrift:')>>
  final public async function processAsync(
    TProtocol $input,
    TProtocol $output,
    ?string $fname = null,
    ?int $rseqid = null,
  ): Awaitable<bool> {
    if ($fname === null || $rseqid === null) {
      $rseqid = 0;
      $fname = '';
      $mtype = 0;

      $input->readMessageBegin(inout $fname, inout $mtype, inout $rseqid);
    }

    HH\set_frame_metadata(nameof static.':'.$fname);
    if (!$this->isSubRequest()) {
      RelativeScript::setMinorPath($fname);
    }
    $methodname = 'process_'.$fname;
    if (JustKnobs::eval('www/mas_buenopath:enable_mbp_thrift')) {
      self::setBuenopath(ThriftMasBuenopath::newBuilder(
        shape("class_name" => static::class, "method_name" => $methodname),
      ));
    }
    if (!PHP\method_exists($this, $methodname)) {
      $handler_ctx = $this->eventHandler_->getHandlerContext($fname);
      $this->eventHandler_->preRead($handler_ctx, $fname, dict[]);
      $input->skip(TType::STRUCT);
      $input->readMessageEnd();
      $this->eventHandler_->postRead($handler_ctx, $fname, dict[]);
      $x = TApplicationException::fromShape(shape(
        'message' => 'Function '.$fname.' not implemented.',
        'code' => TApplicationException::UNKNOWN_METHOD,
      ));
      $this->eventHandler_->handlerError($handler_ctx, $fname, $x);
      $output->writeMessageBegin($fname, TMessageType::EXCEPTION, $rseqid);
      $x->write($output);
      $output->writeMessageEnd();
      $output->getTransport()->flush();
      return true;
    }
    /* HH_FIXME[2011]: This is safe */
    await $this->$methodname($rseqid, $input, $output);
    return true;
  }

  public function process(
    TProtocol $input,
    TProtocol $output,
    ?string $fname = null,
    ?int $rseqid = null,
  ): bool {
    return Asio::awaitSynchronously(
      $this->processAsync($input, $output, $fname, $rseqid),
    );
  }
}
