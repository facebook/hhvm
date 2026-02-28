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

package com.facebook.thrift.util;

import com.facebook.swift.service.ContextChain;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.payload.Writer;
import org.apache.thrift.ErrorBlame;
import org.apache.thrift.ErrorClassification;
import org.apache.thrift.PayloadAppUnknownExceptionMetdata;
import org.apache.thrift.PayloadDeclaredExceptionMetadata;
import org.apache.thrift.PayloadExceptionMetadata;
import org.apache.thrift.PayloadExceptionMetadataBase;
import org.apache.thrift.PayloadMetadata;
import org.apache.thrift.PayloadResponseMetadata;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;

public final class RpcPayloadUtil {
  public static final TStruct TSTRUCT = new TStruct();
  private static final String RESPONSE_FIELD_NAME = "responseField";
  private static final short RESPONSE_FIELD_ID = 0;
  public static final TField VOID_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.VOID, RESPONSE_FIELD_ID);
  public static final TField BOOL_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.BOOL, RESPONSE_FIELD_ID);
  public static final TField BYTE_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.BYTE, RESPONSE_FIELD_ID);
  public static final TField DOUBLE_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.DOUBLE, RESPONSE_FIELD_ID);
  public static final TField I16_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.I16, RESPONSE_FIELD_ID);
  public static final TField I32_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.I32, RESPONSE_FIELD_ID);
  public static final TField I64_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.I64, RESPONSE_FIELD_ID);
  public static final TField STRING_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.STRING, RESPONSE_FIELD_ID);
  public static final TField STRUCT_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.STRUCT, RESPONSE_FIELD_ID);
  public static final TField MAP_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.MAP, RESPONSE_FIELD_ID);
  public static final TField SET_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.SET, RESPONSE_FIELD_ID);
  public static final TField LIST_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.LIST, RESPONSE_FIELD_ID);
  public static final TField ENUM_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.ENUM, RESPONSE_FIELD_ID);
  public static final TField FLOAT_FIELD =
      new TField(RESPONSE_FIELD_NAME, TType.FLOAT, RESPONSE_FIELD_ID);

  private RpcPayloadUtil() {}

  public static ServerResponsePayload fromTApplicationException(
      final TApplicationException applicationException,
      final RequestRpcMetadata requestRpcMetadata,
      final ContextChain chain) {
    return ServerResponsePayload.createWithTApplicationException(
        protocol -> {
          try {
            if (chain != null) {
              chain.preWriteException(applicationException);
              chain.undeclaredUserException(applicationException);
              // Application exceptions are sent to client, and the connection can be reused
              applicationException.write(protocol);
              chain.postWriteException(applicationException);
            } else {
              applicationException.write(protocol);
            }
          } catch (Exception e) {
            throw new TException(e);
          }
        },
        new ResponseRpcMetadata.Builder()
            .setPayloadMetadata(
                PayloadMetadata.fromExceptionMetadata(
                    new PayloadExceptionMetadataBase.Builder()
                        .setWhatUtf8(applicationException.getMessage())
                        .setMetadata(
                            PayloadExceptionMetadata.fromAppUnknownException(
                                new PayloadAppUnknownExceptionMetdata.Builder()
                                    .setErrorClassification(
                                        new ErrorClassification.Builder()
                                            .setBlame(ErrorBlame.SERVER)
                                            .build())
                                    .build()))
                        .build()))
            .setOtherMetadata(requestRpcMetadata.getOtherMetadata())
            .build(),
        null,
        false);
  }

  public static ServerResponsePayload createServerResponsePayload(
      final ServerRequestPayload payload, final Writer writer) {
    return createServerResponsePayload(payload, writer, null);
  }

  public static PayloadExceptionMetadata createDeclaredPayloadException() {
    PayloadDeclaredExceptionMetadata metadata =
        new PayloadDeclaredExceptionMetadata.Builder()
            .setErrorClassification(
                new ErrorClassification.Builder().setBlame(ErrorBlame.SERVER).build())
            .build();

    return PayloadExceptionMetadata.fromDeclaredException(metadata);
  }

  public static ServerResponsePayload createServerResponsePayload(
      final ServerRequestPayload payload, final Writer writer, String what) {

    PayloadExceptionMetadataBase base = null;
    if (what != null) {
      base =
          new PayloadExceptionMetadataBase.Builder()
              .setWhatUtf8(what)
              .setMetadata(createDeclaredPayloadException())
              .build();
    }

    return ServerResponsePayload.create(
        writer, createResponseRpcMetadata(payload.getRequestRpcMetadata(), base), null, false);
  }

  private static ResponseRpcMetadata createResponseRpcMetadata(
      final RequestRpcMetadata requestRpcMetadata, PayloadExceptionMetadataBase base) {
    final PayloadMetadata payloadMetadata;
    if (base != null) {
      payloadMetadata = PayloadMetadata.fromExceptionMetadata(base);
    } else {
      payloadMetadata =
          PayloadMetadata.fromResponseMetadata(PayloadResponseMetadata.defaultInstance());
    }

    return new ResponseRpcMetadata.Builder()
        .setPayloadMetadata(payloadMetadata)
        .setOtherMetadata(requestRpcMetadata.getOtherMetadata())
        .build();
  }
}
