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

namespace java.swift test.fixtures.optionals

include "thrift/annotation/thrift.thrift"

struct Color {
  1: double red;
  2: double green;
  3: double blue;
  4: double alpha;
}

enum Animal {
  DOG = 1,
  CAT = 2,
  TARANTULA = 3,
}

struct Vehicle {
  1: Color color;
  2: optional string licensePlate;
  3: optional string description;
  4: optional string name;
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  5: optional bool hasAC = false;
}

typedef i64 PersonID

struct Person {
  1: PersonID id;
  2: string name;
  3: optional i16 age;
  4: optional string address;
  5: optional Color favoriteColor;
  6: optional set<PersonID> friends;
  7: optional PersonID bestFriend;
  8: optional map<Animal, string> petNames;
  9: optional Animal afraidOfAnimal;
  10: optional list<Vehicle> vehicles;
}
