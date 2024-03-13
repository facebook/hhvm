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

include "thrift/annotation/cpp.thrift"

namespace cpp2 facebook.sbe.test

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf

struct TSingleCustomerLookup {
  1: string customerId;
}

struct TMultipleCustomerLookup {
  1: list<string> customerIds;
}

struct TCustomerResponse {
  1: i64 index;
  2: string customerId;
  3: string firstName;
  4: string lastName;
  5: string company;
  6: string city;
  7: string country;
  8: string phone1;
  9: string phone2;
  10: string email;
  11: string subscriptionDate;
  12: string webSite;
}

struct TMultipleCustomerResponse {
  1: list<TCustomerResponse> customerResponses;
}

service CustomerLookupService {
  IOBuf lookupOne(1: IOBuf request);
  IOBuf lookupMany(1: IOBuf request);
  TCustomerResponse lookupOneT(1: TSingleCustomerLookup request);
  TMultipleCustomerResponse lookupManyT(1: TMultipleCustomerLookup request);

  @cpp.ProcessInEbThreadUnsafe
  IOBuf lookupOneE(1: IOBuf request);
  @cpp.ProcessInEbThreadUnsafe
  IOBuf lookupManyE(1: IOBuf request);
  @cpp.ProcessInEbThreadUnsafe
  TCustomerResponse lookupOneTE(1: TSingleCustomerLookup request);
  @cpp.ProcessInEbThreadUnsafe
  TMultipleCustomerResponse lookupManyTE(1: TMultipleCustomerLookup request);
}
