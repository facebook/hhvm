/*
 * Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/src/meta_schema_validator.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"

namespace xpl {
namespace test {

using ::testing::HasSubstr;

class Meta_schema_validator_test : public ::testing::Test {
 public:
  Meta_schema_validator validator{5};
};

TEST_F(Meta_schema_validator_test, empty_schema) {
  const auto error = validator.validate("");
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA, error);
  ASSERT_THAT(error.message, HasSubstr("is not a valid JSON document"));
}

TEST_F(Meta_schema_validator_test, empty_array_schema) {
  const auto error = validator.validate("[]");
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA, error);
  ASSERT_THAT(error.message, HasSubstr("is not a valid JSON object"));
}

TEST_F(Meta_schema_validator_test, empty_object_schema) {
  const auto error = validator.validate("{}");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, simple_schema) {
  const auto error = validator.validate(R"({
    "type": "object"
  })");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, typo_schema) {
  const auto error = validator.validate(R"({
    "typeE": "object"
  })");
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA, error);
  ASSERT_THAT(error.message,
              HasSubstr("#/typeE failed requirement: 'additionalProperties'"));
}

TEST_F(Meta_schema_validator_test, valid_reference_schema) {
  const auto error = validator.validate(R"({
    "definitions": {
      "veggie": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "like": { "type": "boolean" }
        }
      }
    },
    "properties": {
      "vegetables": {
        "type": "array",
        "items": { "$ref": "#/definitions/veggie" }
      }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, not_valid_reference_schema) {
  const auto error = validator.validate(R"({
    "definitions": {
      "veggie": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "like": { "type": "boolean" }
        }
      }
    },
    "properties": {
      "vegetables": {
        "type": "array",
        "items": { "$ref": "#/definitions/veggieE" }
      }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA, error);
  ASSERT_THAT(
      error.message,
      HasSubstr("reference '#/properties/vegetables/items' is not valid"));
}

TEST_F(Meta_schema_validator_test, reference_to_reference_schema) {
  const auto error = validator.validate(R"({
    "definitions": {
      "veggie": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "like": { "type": "boolean" }
        }
      },
      "veggie_array": {
        "type": "array",
        "items": { "$ref": "#/definitions/veggie" }
      }
    },
    "properties": {
      "vegetables": { "$ref": "#/definitions/veggie_array" }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, self_reference_schema) {
  const auto error = validator.validate(R"({
    "definitions": {
      "veggie": {
        "type": "object",
        "properties": {
          "veggie": { "$ref": "#/definitions/veggie" }
        }
      }
    },
    "properties": {
      "vegetables": { "$ref": "#/definitions/veggie" }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, quick_self_reference_schema) {
  const auto error = validator.validate(R"({
    "properties": {
      "vegetables": { "$ref": "#" }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, malformed_reference_schema) {
  const auto error = validator.validate(R"({
    "properties": {
      "vegetables": { "$ref": "#bad" }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA, error);
  ASSERT_THAT(error.message,
              HasSubstr("reference '#/properties/vegetables' is not valid"));
}

TEST_F(Meta_schema_validator_test, circular_reference_schema) {
  const auto error = validator.validate(R"({
    "definitions": {
      "veggie": {
        "type": "object",
        "properties": {
          "fruit": { "$ref": "#/definitions/fruttie" }
        }
      },
      "fruttie": {
        "type": "object",
        "properties": {
          "vegetable": { "$ref": "#/definitions/veggie" }
        }
      }
    },
    "properties": {
      "fruit": { "$ref": "#/definitions/fruttie" }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Meta_schema_validator_test, too_deep_schema) {
  const auto error = validator.validate(R"({
    "properties": {
      "veggie": {
        "type": "object",
        "properties": {
          "name": {
            "type": "object",
            "properties": {
               "native": { "type": "string" },
               "translation": { "type": "string" }
            }
          }
        }
      }
    }
  })");
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA, error);
  ASSERT_THAT(
      error.message,
      HasSubstr("exceeds the maximum depth on"
                " #/properties/veggie/properties/name/properties/native"));
}

TEST_F(Meta_schema_validator_test, meta_schema) {
  const auto error =
      validator.validate(Meta_schema_validator::k_reference_schema);
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

}  // namespace test
}  // namespace xpl
