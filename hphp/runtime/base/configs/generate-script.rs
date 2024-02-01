/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

/*

This is the script that parses the configuration specification. The format is
following:

```
Any comments until we hit the first section

# SectionName

  Optional section comment paragraph that should be indented 2 spaces

- Type ConfigName[ = DefaultValue], Owner[, feature1|feature2...]

  Optional config comment paragraph that should be indented 2 spaces

```

Supported Types can be found in ConfigType

The ConfigName always need to be prefixed with the `SectionName.`. This is
done to make it easier to search

If no default value is applied you need to implement a custom function that
returns the default value. You can look at php7-custom.cpp for an example.

If the Owner is unknown use `UNKNOWN` as the owner.

*/

use std::collections::HashSet;
use std::fs;
use std::path::PathBuf;
use std::process::ExitCode;

use clap::Parser;
use nom::branch::alt;
use nom::bytes::complete::tag;
use nom::bytes::complete::take_until;
use nom::character::complete::alphanumeric1;
use nom::character::complete::newline;
use nom::character::complete::not_line_ending;
use nom::character::complete::space0;
use nom::character::complete::space1;
use nom::combinator::all_consuming;
use nom::combinator::map;
use nom::combinator::opt;
use nom::combinator::recognize;
use nom::combinator::value;
use nom::multi::count;
use nom::multi::many0;
use nom::multi::many1;
use nom::multi::separated_list1;
use nom::sequence::delimited;
use nom::sequence::preceded;
use nom::sequence::tuple;
use nom::IResult;

#[derive(Debug, PartialEq, Clone, Eq, Hash)]
pub enum ConfigFeature {
    RepoSpecific,
    RequestLevel,
    Private,
    GlobalData,
    UnitCacheFlag,
    RepoOptionsFlag,
}

#[derive(Debug, PartialEq)]
pub struct Config {
    type_: ConfigType,
    pub name: String,
    pub default_value: Option<String>,
    pub owner: Option<String>,
    pub description: Option<String>,
    pub features: HashSet<ConfigFeature>,
}

impl Config {
    fn type_str(&self) -> &str {
        self.type_.str()
    }

    fn shortname(&self) -> String {
        self.name[self.name.find('.').unwrap() + 1..]
            .to_owned()
            .replace('.', "")
    }
}

#[derive(Debug, PartialEq, Clone)]
pub enum ConfigType {
    Bool,
    Int,
    Int64t,
    UInt32t,
    StdString,
    StdVectorStdString,
    BoostFlatSetStdString,
}

impl ConfigType {
    fn str(&self) -> &str {
        match *self {
            ConfigType::Bool => "bool",
            ConfigType::Int => "int",
            ConfigType::Int64t => "int64_t",
            ConfigType::UInt32t => "uint32_t",
            ConfigType::StdString => "std::string",
            ConfigType::StdVectorStdString => "std::vector<std::string>",
            ConfigType::BoostFlatSetStdString => "boost::container::flat_set<std::string>",
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct ConfigSection {
    pub name: String,
    pub description: Option<String>,
    pub configs: Vec<Config>,
}

#[derive(Debug, Parser)]
#[clap(name = "HHVM Generate Configs")]
struct Arguments {
    output_dir: PathBuf,
    input: PathBuf,
}

fn parse_type(input: &str) -> IResult<&str, ConfigType> {
    preceded(
        space0,
        alt((
            value(ConfigType::Bool, tag("bool")),
            value(ConfigType::Int64t, tag("int64_t")),
            value(ConfigType::Int, tag("int")),
            value(ConfigType::UInt32t, tag("uint32_t")),
            value(ConfigType::StdString, tag("std::string")),
            value(
                ConfigType::StdVectorStdString,
                tag("std::vector<std::string>"),
            ),
            value(
                ConfigType::BoostFlatSetStdString,
                tag("boost::container::flat_set<std::string>"),
            ),
        )),
    )(input)
}

fn parse_name(input: &str) -> IResult<&str, &str> {
    preceded(
        space0,
        recognize(tuple((
            alphanumeric1,
            many1(preceded(tag("."), alphanumeric1)),
        ))),
    )(input)
}

fn parse_default_value(input: &str) -> IResult<&str, Option<&str>> {
    opt(preceded(tuple((space0, tag("="), space1)), take_until(",")))(input)
}

fn parse_owner(input: &str) -> IResult<&str, Option<&str>> {
    map(
        preceded(tuple((space0, tag(","), space0)), alphanumeric1),
        |s: &str| {
            if s == "UNKNOWN" {
                return None;
            }
            Some(s)
        },
    )(input)
}

fn parse_features(input: &str) -> IResult<&str, HashSet<ConfigFeature>> {
    map(
        opt(preceded(
            tuple((space0, tag(","), space0)),
            separated_list1(
                tag("|"),
                alt((
                    value(ConfigFeature::RepoSpecific, tag("repospecific")),
                    value(ConfigFeature::RequestLevel, tag("request")),
                    value(ConfigFeature::Private, tag("private")),
                    value(ConfigFeature::GlobalData, tag("globaldata")),
                    value(ConfigFeature::UnitCacheFlag, tag("unitcacheflag")),
                    value(ConfigFeature::RepoOptionsFlag, tag("repooptionsflag")),
                )),
            ),
        )),
        |v| {
            let mut s = HashSet::from_iter(v.unwrap_or(vec![]));
            if s.contains(&ConfigFeature::RepoOptionsFlag) {
                s.insert(ConfigFeature::Private);
            }
            s
        },
    )(input)
}

fn parse_description(input: &str) -> IResult<&str, Option<String>> {
    alt((
        preceded(
            count(newline, 2),
            opt(map(
                many1(delimited(tag("  "), not_line_ending, newline)),
                |s: Vec<&str>| s.join("\n"),
            )),
        ),
        value(None, many0(newline)),
    ))(input)
}

fn parse_config(input: &str) -> IResult<&str, Config> {
    map(
        preceded(
            tuple((opt(newline), tag("-"), space1)),
            tuple((
                parse_type,
                parse_name,
                parse_default_value,
                parse_owner,
                parse_features,
                parse_description,
            )),
        ),
        |(type_, name, default_value, owner, features, description)| Config {
            type_,
            name: name.to_string(),
            default_value: default_value.map(|s| s.to_string()),
            owner: owner.map(|s| s.to_string()),
            features,
            description,
        },
    )(input)
}

fn parse_section(input: &str) -> IResult<&str, ConfigSection> {
    map(
        tuple((
            preceded(tuple((opt(newline), tag("#"), space1)), alphanumeric1),
            parse_description,
            many1(parse_config),
        )),
        |(name, description, configs)| ConfigSection {
            name: name.to_string(),
            description,
            configs,
        },
    )(input)
}

fn parse_option_doc(input: &str) -> IResult<&str, Vec<ConfigSection>> {
    all_consuming(preceded(take_until("#"), many1(parse_section)))(input)
}

fn generate_files(sections: Vec<ConfigSection>, output_dir: PathBuf) {
    let mut repo_global_data = vec![];
    let mut unit_cache_flags = vec![];
    let mut repo_options_flags = vec![];
    let mut repo_options_sections = vec![];

    for section in sections.iter() {
        let lower_section_name = section.name.to_lowercase();

        let has_repo_option_flags = section
            .configs
            .iter()
            .filter(|c| c.features.contains(&ConfigFeature::RepoOptionsFlag))
            .count()
            > 0;

        if has_repo_option_flags {
            repo_options_sections.push(format!("  S({})\\", section.name));
        }

        // [section].h file
        let mut public_defs = vec![];
        let mut public_methods = vec![];
        let mut private_defs = vec![];
        let mut private_methods = vec![];
        for config in section.configs.iter() {
            if config.features.contains(&ConfigFeature::Private) {
                private_defs.push(format!(
                    "  static {} {};",
                    config.type_str(),
                    config.shortname()
                ));
            } else {
                public_defs.push(format!(
                    "  static {} {};",
                    config.type_str(),
                    config.shortname()
                ));
            }
            if config.default_value.is_none() {
                private_methods.push(format!(
                    "  static {} {}Default();\n",
                    config.type_str(),
                    config.shortname()
                ));
            }
        }

        public_methods.push("  static void Load(const IniSettingMap& ini, const Hdf& config);");
        if has_repo_option_flags {
            public_methods.push("  static void GetRepoOptionsFlags(RepoOptionsFlags& flags);");
            public_methods.push("  static void GetRepoOptionsFlagsFromConfig(RepoOptionsFlags& flags, const Hdf& config);");
        }

        let h_content = format!(
            r#"#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <boost/container/flat_set.hpp>

namespace HPHP {{

struct IniSettingMap;
struct Hdf;
struct RepoOptionsFlags;

namespace Cfg {{

struct {} {{

{}

{}

private:
{}

{}
}};

}} // namespace Cfg

}} // namespace HPHP
"#,
            section.name,
            public_defs.join("\n"),
            public_methods.join("\n"),
            private_defs.join("\n"),
            private_methods.join("\n"),
        );
        let mut h_output_file = output_dir.clone();
        h_output_file.push(format!("{}.h", lower_section_name));
        fs::write(h_output_file, h_content).unwrap();

        // [section].cpp file
        let mut defs = vec![];
        let mut bind_calls = vec![];
        let mut repo_options_flags_calls = vec![];
        let mut repo_options_flags_from_config_calls = vec![];
        for config in section.configs.iter() {
            let name = config.shortname();
            defs.push(format!("{} {}::{};", config.type_str(), section.name, name));
            bind_calls.push(format!(
                r#"  Config::Bind({}, ini, config, "{}", {});"#,
                name,
                config.name,
                config
                    .default_value
                    .clone()
                    .unwrap_or(format!("{}Default()", name)),
            ));

            if config.features.contains(&ConfigFeature::RepoOptionsFlag) {
                repo_options_flags_calls.push(format!("  flags.{} = {};", name, name));
                repo_options_flags_from_config_calls.push(format!(
                    r#"  hdfExtract(config, "Parser." "{}", flags.{}, {});"#,
                    config.name, name, name
                ));
            }
        }

        let mut repo_options_methods = String::new();
        if has_repo_option_flags {
            repo_options_methods = format!(
                r#"void {}::GetRepoOptionsFlags(RepoOptionsFlags& flags) {{
{}
}}

void {}::GetRepoOptionsFlagsFromConfig(RepoOptionsFlags& flags, const Hdf& config) {{
{}
}}

"#,
                section.name,
                repo_options_flags_calls.join("\n"),
                section.name,
                repo_options_flags_from_config_calls.join("\n")
            );
        }

        let cpp_content = format!(
            r#"#include "hphp/runtime/base/configs/{}.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/hdf-extract.h"

namespace HPHP::Cfg {{

{}

void {}::Load(const IniSetting::Map& ini, const Hdf& config) {{
{}
}}

{}}} // namespace HPHP::Cfg
"#,
            lower_section_name,
            defs.join("\n"),
            section.name,
            bind_calls.join("\n"),
            &repo_options_methods,
        );

        let mut cpp_output_file = output_dir.clone();
        cpp_output_file.push(format!("{}.cpp", lower_section_name));
        fs::write(cpp_output_file, cpp_content).unwrap();

        // macro files
        for config in section.configs.iter() {
            let name = config.shortname();
            if config.features.contains(&ConfigFeature::GlobalData) {
                repo_global_data.push(format!(
                    "  C(Cfg::{}::{}, {}_{}, {})\\",
                    section.name,
                    name,
                    section.name,
                    name,
                    &config.type_str(),
                ));
            }
            if config.features.contains(&ConfigFeature::UnitCacheFlag) {
                unit_cache_flags.push(format!(
                    "  C(Cfg::{}::{}, {}_{}, {})\\",
                    section.name,
                    name,
                    section.name,
                    name,
                    &config.type_str(),
                ));
            }
            if config.features.contains(&ConfigFeature::RepoOptionsFlag) {
                repo_options_flags.push(format!("  C({}, {})\\", &config.type_str(), name,));
            }
        }
    }

    // repo-global-data-generated.h
    let repo_global_data_content = format!(
        r#"#pragma once
#define CONFIGS_FOR_REPOGLOBALDATA() \
{}

"#,
        repo_global_data.join("\n")
    );
    let mut repo_global_data_file = output_dir.clone();
    repo_global_data_file.push("repo-global-data-generated.h");
    fs::write(repo_global_data_file, repo_global_data_content).unwrap();

    // unit-cache-generated
    let unit_cache_content = format!(
        r#"#pragma once

#define CONFIGS_FOR_UNITCACHEFLAGS() \
{}

"#,
        unit_cache_flags.join("\n")
    );
    let mut unit_cache_file = output_dir.clone();
    unit_cache_file.push("unit-cache-generated.h");
    fs::write(unit_cache_file, unit_cache_content).unwrap();

    // repo-option-flags-generated.h
    let repo_options_flags_content = format!(
        r#"#pragma once

#define CONFIGS_FOR_REPOOPTIONSFLAGS() \
{}

#define SECTIONS_FOR_REPOOPTIONSFLAGS() \
{}

"#,
        repo_options_flags.join("\n"),
        repo_options_sections.join("\n"),
    );
    let mut repo_option_flags_file = output_dir.clone();
    repo_option_flags_file.push("repo-options-flags-generated.h");
    fs::write(repo_option_flags_file, repo_options_flags_content).unwrap();

    let mut config_load_includes = vec![];
    let mut config_load_calls = vec![];
    let mut repo_options_flags_calls = vec![];
    let mut repo_options_flags_from_config_calls = vec![];

    for section in sections.iter() {
        let lower_section_name = section.name.to_lowercase();
        config_load_includes.push(format!(
            r#"#include "hphp/runtime/base/configs/{}.h""#,
            lower_section_name
        ));
        config_load_calls.push(format!("  Cfg::{}::Load(ini, config);", section.name));

        if section
            .configs
            .iter()
            .filter(|c| c.features.contains(&ConfigFeature::RepoOptionsFlag))
            .count()
            > 0
        {
            repo_options_flags_calls.push(format!(
                "  Cfg::{}::GetRepoOptionsFlags(flags);",
                section.name,
            ));
            repo_options_flags_from_config_calls.push(format!(
                "  Cfg::{}::GetRepoOptionsFlagsFromConfig(flags, config);",
                section.name,
            ));
        }
    }

    let configs_load_content = format!(
        r#"#include "hphp/runtime/base/configs/configs-load.h"

{}

namespace HPHP {{

struct IniSettingMap;
struct Hdf;
struct RepoOptionsFlags;

namespace Cfg {{

void Load(const IniSettingMap& ini, const Hdf& config) {{
{}
}}

void GetRepoOptionsFlags(RepoOptionsFlags& flags) {{
{}
}}

void GetRepoOptionsFlagsFromConfig(RepoOptionsFlags& flags, const Hdf& config) {{
{}
}}

}} // namespace Cfg

}} // namespace HPHP
"#,
        config_load_includes.join("\n"),
        config_load_calls.join("\n"),
        repo_options_flags_calls.join("\n"),
        repo_options_flags_from_config_calls.join("\n"),
    );
    let mut configs_load_file = output_dir.clone();
    configs_load_file.push("configs-load-generated.cpp");
    fs::write(configs_load_file, configs_load_content).unwrap();
}

fn main() -> ExitCode {
    let args = Arguments::parse();

    println!("OUT {:?} {:?}", args.output_dir, args.input);

    let contents = fs::read_to_string(args.input).expect("Should have been able to read the file");

    let res = parse_option_doc(&contents);
    match res {
        Ok((_, sections)) => {
            generate_files(sections, args.output_dir);
            ExitCode::from(0)
        }
        Err(e) => {
            println!("error parsing header: {e:?}");
            ExitCode::from(1)
        }
    }
}
