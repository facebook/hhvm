import argparse
import re

parser = argparse.ArgumentParser(
    description="Extracts section names from HHVM's config.specification file"
)
parser.add_argument("config_spec", help="path to the configs.specification file")

args = parser.parse_args()

sections = []

with open(args.config_spec, "r") as f:
    for line in f:
        if line[0] != "#":
            continue

        section_name = (
            re.search(r"^#\s*([a-zA-Z0-9\.]+)", line).group(1).replace(".", "").lower()
        )
        sections.append(section_name)

sections.sort()

print(";".join(sections))
