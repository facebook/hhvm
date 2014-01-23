#########################################
# 
# Install all the dependencies for HipHop
#
#########################################

SCRIPT_NAME='./configure_fedora_20.sh'
if [ "$0" != "$SCRIPT_NAME" ]; then
  echo "Run the script from the hiphop-php directory like:"
  echo "  $SCRIPT_NAME"
  exit 1
fi

if [ "x${TRAVIS}" != "x" ]; then
  # Collect some stats for use in tuning build later on
  free
  CPUS=`cat /proc/cpuinfo | grep -E '^processor' | tail -1 | cut -d : -f 2`
  CPUS=`expr ${CPUS} + 1`
  echo "CPUs: ${CPUS}"
fi

export CMAKE_PREFIX_PATH=`/bin/pwd`/..

# install the actual dependencies
sudo yum groupinstall "Fedora Packager" -y
sudo yum install -y git wget make \
    autoconf \
    binutils-devel \
    boost-devel \
    bzip2-devel \
    chrpath \
    cmake \
    cyrus-sasl \
    elfutils-libelf-devel \
    expat-devel \
    fontconfig-devel \
    freetype-devel \
    gcc-c++ \
    gd-devel \
    glibc-devel \
    glog-devel \
    jemalloc-devel \
    keyutils-libs \
    krb5-devel \
    libaio-devel \
    libcap-devel \
    libc-client \
    libc-client-devel \
    libcom_err-devel \
    libdwarf-devel \
    libedit-devel \
    libicu-devel \
    libjpeg-turbo \
    libjpeg-turbo-devel \
    libmcrypt-devel \
    libmemcached-devel \
    libpng-devel \
    libselinux-devel \
    libsepol-devel \
    libstdc++-devel \
    libtool \
    libunwind-devel \
    libvpx-devel \
    libX11-devel \
    libXau-devel \
    libxcb-devel \
    libxml2-devel \
    libxml++-devel \
    libXpm-devel \
    mysql-devel \
    ncurses-devel \
    oniguruma-devel \
    openldap-devel \
    openssl-devel \
    pam-devel \
    pcre-devel \
    readline-devel \
    systemtap-sdt \
    tbb-devel \
    unixODBC-devel \
    uw-imap-devel \
    xorg-x11-devel \
    zlib \
    zlib-devel

git clone git://github.com/libevent/libevent.git --quiet &
git clone git://github.com/bagder/curl.git --quiet &

# init submodules
git submodule update --init

# wait until all background processes finished
FAIL=0

for job in `jobs -p`
do
    echo "waiting for background job $job"
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" == "0" ];
then
    echo "all downloads finished"
else
    echo "$FAIL errors while downloading!"
    exit 100
fi 

# libevent
cd libevent
git checkout release-1.4.14b-stable
cat ../hphp/third_party/libevent-1.4.14.fb-changes.diff | patch -p1
./autogen.sh
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# curl
cd curl
./buildconf
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# cleanup
rm -rf libevent curl

# hphp
cmake .

echo "-------------------------------------------------------------------------"
echo "Done. Now run:"
echo "  CMAKE_PREFIX_PATH=\`pwd\`/.. make"
