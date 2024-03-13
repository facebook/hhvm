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

#pragma once

namespace facebook::sbe::test {

struct Customer {
  std::int64_t index;
  std::string customerId;
  std::string firstName;
  std::string lastName;
  std::string company;
  std::string city;
  std::string country;
  std::string phone1;
  std::string phone2;
  std::string email;
  std::string subscriptionDate;
  std::string website;

  bool operator==(const Customer& other) const {
    return index == other.index && customerId == other.customerId &&
        firstName == other.firstName && lastName == other.lastName &&
        company == other.company && city == other.city &&
        country == other.country && phone1 == other.phone1 &&
        phone2 == other.phone2 && email == other.email &&
        subscriptionDate == other.subscriptionDate && website == other.website;
  }
};

} // namespace facebook::sbe::test
