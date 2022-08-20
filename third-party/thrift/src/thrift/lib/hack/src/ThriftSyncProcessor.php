<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
 * @package thrift
 */

abstract class ThriftSyncProcessor
  extends ThriftProcessorBase {

  abstract const type TThriftIf as IThriftSyncIf;

  public function process(TProtocol $input, TProtocol $output): bool {
    $rseqid = 0;
    $fname = '';
    $mtype = 0;

    $input->readMessageBegin(inout $fname, inout $mtype, inout $rseqid);
    $methodname = 'process_'.$fname;
    if (!method_exists($this, $methodname)) {
      $handler_ctx = $this->eventHandler_->getHandlerContext($fname);
      $this->eventHandler_->preRead($handler_ctx, $fname, array());
      $input->skip(TType::STRUCT);
      $input->readMessageEnd();
      $this->eventHandler_->postRead($handler_ctx, $fname, array());
      $x = new TApplicationException(
        'Function '.$fname.' not implemented.',
        TApplicationException::UNKNOWN_METHOD,
      );
      $this->eventHandler_->handlerError($handler_ctx, $fname, $x);
      $output->writeMessageBegin($fname, TMessageType::EXCEPTION, $rseqid);
      $x->write($output);
      $output->writeMessageEnd();
      $output->getTransport()->flush();
      return true;
    }
    /* UNSAFE_EXPR[2011]: This is safe */
    $this->$methodname($rseqid, $input, $output);
    return true;
  }
}
