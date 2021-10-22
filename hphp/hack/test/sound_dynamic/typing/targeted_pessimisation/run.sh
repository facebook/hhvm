#!/bin/bash
file="$1"
hh_single_type_check="$2"
hh_pessimisation="$3"
targets="$file.log"
pessimised_file="$file.pessimised.php"

echo "# Original file
"
cat -n "$file"
echo "

# Original errors
"
$hh_single_type_check --enable-sound-dynamic-type --hh-log-level pessimise 1 --error-format raw "$file" 1>"$targets"
echo "

# Pessimisation log
"
cat "$targets"
echo "

# Pessimised file
"
$hh_pessimisation --file "$file" --pessimisation-targets "$targets" > "$pessimised_file"
cat -n "$pessimised_file"
echo "

# Errors after pessimisation
"
$hh_single_type_check --like-type-hints --enable-sound-dynamic-type --error-format raw "$pessimised_file"
rm "$targets" "$pessimised_file"
