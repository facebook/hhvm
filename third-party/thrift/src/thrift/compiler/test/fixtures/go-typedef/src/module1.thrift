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

include "module0.thrift"

typedef string Plate
typedef string State
typedef i32 Year
typedef list<string> Drivers
typedef module0.Accessory Accessory
typedef module0.PartName PartName (go.name = "CarPartName")

struct Automobile {
  1: Plate plate;
  2: optional Plate previous_plate;
  3: optional Plate first_plate = "0000";
  4: Year year;
  5: Drivers drivers;
  6: list<Accessory> Accessories;
  7: map<i32, PartName> PartNames;
}

// Test structs as map keys
struct MapKey {
  1: i64 num;
  2: string strval;
}

struct MapContainer {
  1: map<MapKey, string> mapval;
}

typedef Automobile Car

service Finder {
  Automobile byPlate(1: Plate plate);

  Car aliasByPlate(1: Plate plate);

  Plate previousPlate(1: Plate plate);
}

struct Pair {
  1: Automobile automobile;
  2: Car car;
}

struct Collection {
  1: list<Automobile> automobiles;
  2: list<Car> cars;
}
