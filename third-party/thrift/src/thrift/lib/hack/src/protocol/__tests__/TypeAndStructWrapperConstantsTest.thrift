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

include "TypeAndStructWrapperTest.thrift"
include "thrift/annotation/thrift.thrift"

package "meta.com/thrift/wrapper_test"

namespace hack "WrapperTest"

enum Company {
  // Should not have any impact
  @TypeAndStructWrapperTest.AnnotationStruct
  FACEBOOK = 0,
  WHATSAPP = 1,
  OCULUS = 2,
  INSTAGRAM = 3,
}

@thrift.AllowLegacyTypedefUri
typedef Company MyCompany
const MyCompany my_company = FACEBOOK;

struct Internship {
  1: required i32 weeks;
  2: string title;
  3: optional Company employer;
  @TypeAndStructWrapperTest.AnnotationStruct
  4: optional double compensation;
  5: optional string school;
  6: TypeAndStructWrapperTest.i64WithWrapper intern_id;
}

struct SWE {
  1: optional Company employer;
  2: optional double compensation;
}

const Internship instagram = {
  "weeks": 12,
  "title": "Software Engineer",
  "employer": Company.INSTAGRAM,
  "compensation": 1200.0,
  "school": "Monters University",
  "intern_id": 10011,
};

const list<Internship> internList = [
  instagram,
  {
    "weeks": 10,
    "title": "Sales Intern",
    "employer": Company.FACEBOOK,
    "compensation": 1000.0,
    "intern_id": 10013,
  },
];

@TypeAndStructWrapperTest.AnnotationStruct
const list<SWE> engineers = [
  {"employer": Company.FACEBOOK, "compensation": 1000.0},
  {"employer": Company.WHATSAPP, "compensation": 1200.0},
  {"employer": Company.OCULUS, "compensation": 1200.0},
];

const list<TypeAndStructWrapperTest.StructWithWrapper> wrapped_structs = [
  {"int_field": 1},
  {"int_field": 2},
];

const list<TypeAndStructWrapperTest.i64WithWrapper> wrapped_ints = [1, 2];
