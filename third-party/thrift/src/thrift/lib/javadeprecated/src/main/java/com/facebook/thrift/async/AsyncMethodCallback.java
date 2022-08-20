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
 */

package com.facebook.thrift.async;

public interface AsyncMethodCallback {
  /**
   * This method will be called when the remote side has completed invoking your method call and the
   * result is fully read. For oneway method calls, this method will be called as soon as we have
   * completed writing out the request.
   */
  public void onComplete(TAsyncMethodCall response);

  /**
   * This method will be called when there is an unexpected clientside exception. This does not
   * include application-defined exceptions that appear in the IDL, but rather things like
   * IOExceptions.
   */
  public void onError(Exception exception);
}
