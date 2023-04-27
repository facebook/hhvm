/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPTransactionEgressSM.h>

#include <folly/Indestructible.h>

namespace proxygen {

std::pair<HTTPTransactionEgressSMData::State, bool>
HTTPTransactionEgressSMData::find(HTTPTransactionEgressSMData::State s,
                                  HTTPTransactionEgressSMData::Event e) {
  using State = HTTPTransactionEgressSMData::State;
  using Event = HTTPTransactionEgressSMData::Event;

  //             +--> ChunkHeaderSent -> ChunkBodySent
  //             |      ^                    v
  //             |      |   ChunkTerminatorSent -> TrailersSent
  //             |      |__________|        |          |
  //             |                          |          v
  // Start -> HeadersSent                   +----> EOMQueued --> SendingDone
  //             |                                     ^
  //             +------------> RegularBodySent -------+

  static const folly::Indestructible<TransitionTable<State, Event>> transitions{
      TransitionTable<State, Event>{
          static_cast<uint64_t>(State::NumStates),
          static_cast<uint64_t>(Event::NumEvents),
          {{{State::Start, Event::sendHeaders}, State::HeadersSent},

           // For HTTP sending 100 response, then a regular response
           {{State::HeadersSent, Event::sendHeaders}, State::HeadersSent},

           {{State::HeadersSent, Event::sendBody}, State::RegularBodySent},
           {{State::HeadersSent, Event::sendTrailers}, State::TrailersSent},
           {{State::HeadersSent, Event::sendChunkHeader},
            State::ChunkHeaderSent},
           {{State::HeadersSent, Event::sendEOM}, State::EOMQueued},

           {{State::RegularBodySent, Event::sendBody}, State::RegularBodySent},
           {{State::RegularBodySent, Event::sendTrailers}, State::TrailersSent},
           {{State::RegularBodySent, Event::sendEOM}, State::EOMQueued},

           {{State::ChunkHeaderSent, Event::sendBody}, State::ChunkBodySent},

           {{State::ChunkBodySent, Event::sendBody}, State::ChunkBodySent},
           {{State::ChunkBodySent, Event::sendChunkTerminator},
            State::ChunkTerminatorSent},

           {{State::ChunkTerminatorSent, Event::sendChunkHeader},
            State::ChunkHeaderSent},
           {{State::ChunkTerminatorSent, Event::sendTrailers},
            State::TrailersSent},
           {{State::ChunkTerminatorSent, Event::sendEOM}, State::EOMQueued},

           {{State::TrailersSent, Event::sendEOM}, State::EOMQueued},

           {{State::HeadersSent, Event::sendDatagram}, State::DatagramSent},
           {{State::DatagramSent, Event::sendDatagram}, State::DatagramSent},
           {{State::DatagramSent, Event::sendTrailers}, State::TrailersSent},
           {{State::DatagramSent, Event::sendEOM}, State::EOMQueued},

           {{State::EOMQueued, Event::eomFlushed}, State::SendingDone}}}};

  return transitions->find(s, e);
}

std::ostream& operator<<(std::ostream& os,
                         HTTPTransactionEgressSMData::State s) {
  switch (s) {
    case HTTPTransactionEgressSMData::State::Start:
      os << "Start";
      break;
    case HTTPTransactionEgressSMData::State::HeadersSent:
      os << "HeadersSent";
      break;
    case HTTPTransactionEgressSMData::State::DatagramSent:
      os << "DatagramSent";
      break;
    case HTTPTransactionEgressSMData::State::RegularBodySent:
      os << "RegularBodySent";
      break;
    case HTTPTransactionEgressSMData::State::ChunkHeaderSent:
      os << "ChunkHeaderSent";
      break;
    case HTTPTransactionEgressSMData::State::ChunkBodySent:
      os << "ChunkBodySent";
      break;
    case HTTPTransactionEgressSMData::State::ChunkTerminatorSent:
      os << "ChunkTerminatorSent";
      break;
    case HTTPTransactionEgressSMData::State::TrailersSent:
      os << "TrailersSent";
      break;
    case HTTPTransactionEgressSMData::State::EOMQueued:
      os << "EOMQueued";
      break;
    case HTTPTransactionEgressSMData::State::SendingDone:
      os << "SendingDone";
      break;
    case HTTPTransactionEgressSMData::State::NumStates:
      CHECK(false) << "Bad state";
      break;
  }

  return os;
}

std::ostream& operator<<(std::ostream& os,
                         HTTPTransactionEgressSMData::Event e) {
  switch (e) {
    case HTTPTransactionEgressSMData::Event::sendHeaders:
      os << "sendHeaders";
      break;
    case HTTPTransactionEgressSMData::Event::sendDatagram:
      os << "sendDatagram";
      break;
    case HTTPTransactionEgressSMData::Event::sendBody:
      os << "sendBody";
      break;
    case HTTPTransactionEgressSMData::Event::sendChunkHeader:
      os << "sendChunkHeader";
      break;
    case HTTPTransactionEgressSMData::Event::sendChunkTerminator:
      os << "sendChunkTerminator";
      break;
    case HTTPTransactionEgressSMData::Event::sendTrailers:
      os << "sendTrailers";
      break;
    case HTTPTransactionEgressSMData::Event::sendEOM:
      os << "sendEOM";
      break;
    case HTTPTransactionEgressSMData::Event::eomFlushed:
      os << "eomFlushed";
      break;
    case HTTPTransactionEgressSMData::Event::NumEvents:
      CHECK(false) << "Bad event";
      break;
  }

  return os;
}

} // namespace proxygen
