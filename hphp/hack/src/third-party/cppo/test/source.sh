#! /bin/sh -e

echo "# $2"
echo "(*"
cat "$1"
echo "*)"
echo "(*"
echo "   Environment variables:"
echo "     CPPO_FILE=$CPPO_FILE"
echo "     CPPO_FIRST_LINE=$CPPO_FIRST_LINE"
echo "     CPPO_LAST_LINE=$CPPO_LAST_LINE"
echo "*)"
echo "# $3"
