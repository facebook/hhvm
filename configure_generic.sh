#!/usr/bin/env bash

#######################################
# Generic Dependency Installer Script #
#######################################

# Make sure the script is run from its proper path.
# OR Since we know the path, why don't we do it ourselves ?
SCRIPT="$(basename $0)"
pushd "$(dirname $0)" >/dev/null
SCRIPT_DIR="$(pwd -P)"
popd >/dev/null
PWD="$(pwd -P)"
if [ $PWD != $SCRIPT_DIR ]
then
    echo "Run the script from the HHVM directory like:"
    echo "cd $SCRIPT_DIR && ./$SCRIPT"
    exit 1
fi

# Identify the Distro
OS_TYPE=$(uname)
if [ "x$OS_TYPE" = "xLinux" ];then
    DISTRO_NAME=$(head -1 /etc/issue)
    CPUS=$(egrep '(^CPU|processor.*:.*)' /proc/cpuinfo|wc -l)
    if grep -i fedora /etc/issue >/dev/null 2>&1; then
        DISTRO=fedora
    elif grep -i ubuntu /etc/issue >/dev/null 2>&1;then
        DISTRO=ubuntu
    else
        DISTRO=unknown
    fi
else
    echo "Linux is the only supported Operating system right now."
    echo "Please submit a PR when this script is enhanced to support"
    echo "other operating systems"
    echo ""
    echo "Project URL: https://github.com/facebook/hhvm"
    echo ""
    echo " - Thank You."
    exit 1
fi

echo "Date/Time      : $(date)"
echo "Current Distro : $DISTRO_NAME"

# For travis
if [ "x${TRAVIS}" != "x" ]; then
  # Collect some stats for use in tuning build later on
  more /proc/meminfo
  vmstat
  free -t -m
  echo "Travis Mode : YES"
  echo "# CPUs      : ${CPUS}"
  echo ""
else
  echo "Travis Mode : NO"
  echo "# CPUs      : ${CPUS}"
  echo ""
fi

# Place to save all the binaries/libraries/headers from the ext packages
export CMAKE_PREFIX_PATH=`/bin/pwd`/..

case $DISTRO in
    fedora)
        # install the actual dependencies
        sudo yum groupinstall "Fedora Packager" -yq
        sudo yum install -yq git wget make autoconf binutils-devel \
            boost-devel bzip2-devel chrpath cmake cyrus-sasl elfutils-libelf-devel  \
            expat-devel fontconfig-devel freetype-devel gcc-c++ gd-devel glibc-devel  \
            glog-devel jemalloc-devel keyutils-libs krb5-devel libaio-devel libcap-devel  \
            libc-client libc-client-devel libcom_err-devel libdwarf-devel libedit-devel \
            libicu-devel libjpeg-turbo libjpeg-turbo-devel libmcrypt-devel libmemcached-devel \
            libpng-devel libselinux-devel libsepol-devel libstdc++-devel libtool \
            libunwind-devel libvpx-devel libX11-devel libXau-devel libxcb-devel libxml2-devel \
            libxml++-devel libXpm-devel mysql-devel ncurses-devel oniguruma-devel openldap-devel \
            openssl-devel pam-devel pcre-devel readline-devel tbb-devel unixODBC-devel \
            uw-imap-devel zlib zlib-devel \
            ImageMagick-devel libxslt-devel &

        # For patched stuff.
        git clone --depth=1 --branch=release-2.0.22-stable git://github.com/libevent/libevent.git --quiet &
        git clone --depth=1 --branch=curl-7_41_0 git://github.com/bagder/curl.git --quiet &
        ;;
    ubuntu)
        # install python-software-properties before trying to add a PPA
        sudo apt-get install -yqq python-software-properties
        sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
        sudo add-apt-repository -y ppa:boost-latest/ppa
        sudo apt-get -yqq update

        # Leave this install till after the main parallel package install above
        # since it adds a non-12.04 package repo and we don't want to
        # pull EVERYTHING in, just the newer gcc compiler (and toolchain)
        GCC_VER=4.9
        if [ "$CXX" = "g++" ]; then
            sudo apt-get -yqq install --force-yes gcc-${GCC_VER} g++-${GCC_VER} libstdc++-${GCC_VER}-dev
            sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VER} 60 \
                                     --slave   /usr/bin/g++ g++ /usr/bin/g++-${GCC_VER}
            sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 40 \
                                     --slave   /usr/bin/g++ g++ /usr/bin/g++-4.6
            sudo update-alternatives --set gcc /usr/bin/gcc-${GCC_VER}
        fi

        # LLVM testing
        if [ "$CXX" = "clang++" ]; then
            sudo apt-get install -yqq clang-3.4 libstdc++-4.8-dev
        fi

        # install the actual dependencies
        sudo apt-get -yqq install git-core cmake boost1.55 libmysqlclient-dev \
            libxml2-dev libmcrypt-dev libicu-dev openssl build-essential binutils-dev \
            libcap-dev libgd2-xpm-dev zlib1g-dev libtbb-dev libonig-dev libpcre3-dev \
            wget memcached libreadline-dev libncurses-dev libmemcached-dev libbz2-dev \
            libc-client2007e-dev php5-mcrypt php5-imagick libgoogle-perftools-dev \
            libcloog-ppl0 libelf-dev libdwarf-dev libunwind7-dev subversion \
            autoconf libtool libcurl4-openssl-dev \
            libmagickwand-dev libxslt1-dev &

        git clone --depth=1 --branch=release-2.0.22-stable git://github.com/libevent/libevent.git --quiet &
        git clone --depth=1 --branch=curl-7_41_0 git://github.com/bagder/curl.git --quiet &
        svn checkout http://google-glog.googlecode.com/svn/trunk/ google-glog --quiet &
        git clone --depth=1 --branch=3.6.0 git://github.com/jemalloc/jemalloc.git --quiet &
        ;;
    *)
        echo "Unknown distribution. Please update packages in this section."
        exit 1
        ;;
esac

# init submodules
git submodule update --init --recursive

# wait until all background processes finished
FAIL=0

for job in `jobs -p`
do
    echo "waiting for background job $job"
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" == "0" ]; then
    echo "all downloads finished"
else
    echo "$FAIL errors while downloading!"
    exit 100
fi

# libevent
cd libevent
./autogen.sh
./configure --prefix=$CMAKE_PREFIX_PATH --disable-dependency-tracking --disable-debug-mode
make -j $CPUS
make install
cd ..

# curl
cd curl
./buildconf
./configure --prefix=$CMAKE_PREFIX_PATH --disable-dependency-tracking --disable-debug --disable-silent-rules
make -j $CPUS
make install
cd ..

if [[ "x$DISTRO" == "xubuntu" ]]; then
  # glog
  cd google-glog
  ./configure --prefix=$CMAKE_PREFIX_PATH --disable-dependency-tracking
  make -j $CPUS
  make install
  cd ..

  # jemaloc
  cd jemalloc
  ./autogen.sh --prefix=$CMAKE_PREFIX_PATH --disable-debug
  make -j $CPUS install_bin install_include install_lib
  cd ..

  # cleanup
  rm -rf google-glog jemalloc
fi

# cleanup
rm -rf libevent curl

# hphp
cmake "$@" .

# all set
echo "-------------------------------------------------------------------------"
echo "Done. Now run:"
echo "  CMAKE_PREFIX_PATH=\`pwd\`/.. make"
