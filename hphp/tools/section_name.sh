#/bin/sh

EXTNAME=$1

# Some platforms limit section names to 16 characters
# So we'll use 'ext.' plus the first 12 characters of md5($extname)
if builtin command -v md5 > /dev/null; then
  SECTNAME=`echo -n ${EXTNAME} | md5 | cut -c 1-12`
elif builtin command -v md5sum > /dev/null; then
  SECTNAME=`echo -n ${EXTNAME} | md5sum | awk '{print $1}' | cut -c 1-12`
else
  echo "Neither md5 nor md5sum available on this system" 2>&1
  exit 1
fi
echo "ext.${SECTNAME}"
