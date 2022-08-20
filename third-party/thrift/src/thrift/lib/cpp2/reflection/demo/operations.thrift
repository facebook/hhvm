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
 */

namespace cpp2 static_reflection.demo

struct create_entity_response {
  1: i64 entity_id;
  2: i64 audit_id;
}

struct query_entity_response {
  1: i64 entity_id;
}

struct delete_entity_response {
  1: i64 audit_id;
}

struct add_field_response {
  1: i64 field_id;
  2: i64 audit_id;
}

struct query_field_response {
  1: i64 field_id;
}

struct delete_field_response {
  1: i64 audit_id;
}

union response_variant {
  1: create_entity_response create_entity;
  2: query_entity_response query_entity;
  3: delete_entity_response delete_entity;
  4: add_field_response add_field;
  5: query_field_response query_field;
  6: delete_field_response delete_field;
}

const response_variant create_entity = {
  "create_entity": {"entity_id": 12345, "audit_id": 11111},
};

const response_variant query_entity = {"query_entity": {"entity_id": 23456}};

const response_variant delete_entity = {"delete_entity": {"audit_id": 22222}};

const response_variant add_field = {
  "add_field": {"field_id": 34567, "audit_id": 33333},
};

const response_variant query_field = {"query_field": {"field_id": 45678}};

const response_variant delete_field = {"delete_field": {"audit_id": 44444}};
