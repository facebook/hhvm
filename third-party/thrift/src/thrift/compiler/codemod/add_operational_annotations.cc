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

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;

namespace {

class add_operational_annotations {
 public:
  add_operational_annotations(source_manager& sm, t_program& program)
      : fm_(sm, program), prog_(program) {}

  void add_includes() {
    // Get the file content
    std::string_view content = fm_.old_content();

    // Check if the include statements already exist
    bool zone_policy_include_exists =
        content.find(
            "include \"dsi/logger/logger_config_fbcode/zone_policy.thrift\"") !=
        std::string::npos;
    bool operational_data_include_exists =
        content.find(
            "include \"configerator/structs/capella/types/annotation_types/operational_data/operational_data_annotation.thrift\"") !=
        std::string::npos;

    // Only add the includes if they don't already exist
    if (!zone_policy_include_exists || !operational_data_include_exists) {
      // Simply add at position 0 (beginning of file)
      std::string includes_to_add;
      if (!zone_policy_include_exists) {
        includes_to_add +=
            "include \"dsi/logger/logger_config_fbcode/zone_policy.thrift\"\n";
      }
      if (!operational_data_include_exists) {
        includes_to_add +=
            "include \"configerator/structs/capella/types/annotation_types/operational_data/operational_data_annotation.thrift\"\n";
      }

      // Check if the next line after our includes contains an include statement
      size_t first_line_end = content.find('\n');
      if (first_line_end != std::string::npos) {
        // Look at the content after the first line
        std::string_view next_content = content.substr(first_line_end + 1);
        // Check if the next content contains an include statement
        bool next_line_has_include =
            next_content.find("include ") != std::string::npos;

        // Add a blank line after the includes only if the next line doesn't
        // have an include
        if (!includes_to_add.empty()) {
          if (!next_line_has_include) {
            includes_to_add += "\n";
          }
          fm_.add({0, 0, includes_to_add});
        }
      } else {
        // If there's no newline (single line file), just add the includes
        if (!includes_to_add.empty()) {
          includes_to_add += "\n";
          fm_.add({0, 0, includes_to_add});
        }
      }
    }
  }
  /*
  This function checks for the universe OPE annotation.
  */
  bool has_operational_annotation() {
    std::string_view content = fm_.old_content();
    return content.find(target_string) != std::string::npos;
  }

  /*
  This function checks for the if the PPF and OPE category annotations already
  exist.
  */
  std::pair<bool, bool> check_existing_annotations(
      std::string_view next_content) {
    bool zone_policy_exists =
        next_content.find(zone_policy_string) != std::string::npos;
    bool operational_data_exists =
        next_content.find(ope_category_string) != std::string::npos;
    return {zone_policy_exists, operational_data_exists};
  }

  /*
  This function adds the PPF and OPE category annotations if one doesn't exist.
  */
  void add_annotations_at(
      size_t line_end,
      const std::string& indentation,
      bool zone_policy_exists,
      bool operational_data_exists) {
    std::string annotations_to_add;
    if (!zone_policy_exists) {
      annotations_to_add += "\n" + indentation + zone_policy_string + "{\n" +
          indentation +
          "  name = purpose_policy_names.TPurposePolicyName.DEFAULT_PURPOSES_OPERATIONAL,\n" +
          indentation +
          "  cipp_enforcement_mode = data_access_policy_metadata.CIPPEnforcementMode.NONE,\n" +
          indentation + "}";
    }
    if (!operational_data_exists) {
      annotations_to_add += "\n" + indentation + ope_category_string + "{\n" +
          indentation +
          "  category = operational_data_annotation.Category.RESTRICTED_DEFAULT,\n" +
          indentation + "}";
    }
    if (!annotations_to_add.empty()) {
      fm_.add({line_end, line_end, annotations_to_add});
    }
  }

  /*
  Calculates identations required for PPF and OPE category annotations.
  */
  std::string get_indentation(std::string_view content, size_t pos) {
    // Get the indentation of the current line
    size_t line_start = content.rfind('\n', pos);
    if (line_start == std::string::npos) {
      line_start = 0;
    } else {
      line_start++; // Move past the newline character
    }

    // Calculate the indentation
    std::string indentation;
    for (size_t i = line_start;
         i < pos && (content[i] == ' ' || content[i] == '\t');
         ++i) {
      indentation += content[i];
    }
    return indentation;
  }

  void visit_program() {
    // Check if the file has operational annotations
    if (has_operational_annotation()) {
      // Add required includes for PPF and OPE cateogry annotations.
      add_includes();

      // Get the file content
      std::string_view content = fm_.old_content();

      // Find all occurrences of the target string
      size_t pos = 0;
      while ((pos = content.find(target_string, pos)) != std::string::npos) {
        // Find the end of the line containing the target string
        size_t line_end = content.find('\n', pos);
        if (line_end == std::string::npos) {
          line_end = content.size();
        }

        // Get the indentation
        std::string indentation = get_indentation(content, pos);

        // Check if the PPF and OPE annotations already exist after this
        // occurrence
        size_t next_pos = line_end + 1;
        std::string_view next_content =
            content.substr(next_pos, 1000); // Look ahead a reasonable amount

        auto [zone_policy_exists, operational_data_exists] =
            check_existing_annotations(next_content);

        // Add annotations if they don't exist
        if (!zone_policy_exists || !operational_data_exists) {
          add_annotations_at(
              line_end,
              indentation,
              zone_policy_exists,
              operational_data_exists);
        }

        // Move past this occurrence
        pos = line_end;
      }
    }
  }

  void run() {
    visit_program();
    fm_.apply_replacements();
  }

 private:
  file_manager fm_;
  const t_program& prog_;
  const std::string target_string =
      "@universe.Universe{id = universe.UniverseIdentifier.OPERATIONAL}";
  const std::string zone_policy_string = "@zone_policy.PurposePolicy";
  const std::string ope_category_string = "@operational_data_annotation.Logger";
};
} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        add_operational_annotations(sm, *pb.root_program()).run();
      });
}
