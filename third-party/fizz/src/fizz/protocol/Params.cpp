/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Params.h>

namespace fizz {
Event EventVisitor::operator()(const Param& param) const {
  switch (param.type()) {
    case Param::Type::ClientHello_E:
      return ClientHello::event;
    case Param::Type::ServerHello_E:
      return ServerHello::event;
    case Param::Type::EndOfEarlyData_E:
      return EndOfEarlyData::event;
    case Param::Type::HelloRetryRequest_E:
      return HelloRetryRequest::event;
    case Param::Type::EncryptedExtensions_E:
      return EncryptedExtensions::event;
    case Param::Type::CertificateRequest_E:
      return CertificateRequest::event;
    case Param::Type::CompressedCertificate_E:
      return CompressedCertificate::event;
    case Param::Type::CertificateMsg_E:
      return CertificateMsg::event;
    case Param::Type::CertificateVerify_E:
      return CertificateVerify::event;
    case Param::Type::Finished_E:
      return Finished::event;
    case Param::Type::NewSessionTicket_E:
      return NewSessionTicket::event;
    case Param::Type::KeyUpdate_E:
      return KeyUpdate::event;
    case Param::Type::Alert_E:
      return Alert::event;
    case Param::Type::CloseNotify_E:
      return CloseNotify::event;
    case Param::Type::Accept_E:
      return Accept::event;
    case Param::Type::Connect_E:
      return Connect::event;
    case Param::Type::AppData_E:
      return AppData::event;
    case Param::Type::AppWrite_E:
      return AppWrite::event;
    case Param::Type::EarlyAppWrite_E:
      return EarlyAppWrite::event;
    case Param::Type::WriteNewSessionTicket_E:
      return WriteNewSessionTicket::event;
    case Param::Type::KeyUpdateInitiation_E:
      return KeyUpdateInitiation::event;
  }
  folly::assume_unreachable();
}
} // namespace fizz
