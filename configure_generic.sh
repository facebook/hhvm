#########################################
#                                       #
# Generic Dependency Installer Script   #
#                                       #
#########################################

# Current Script details
SCRIPT="$(readlink -f ${BASH_SOURCE[0]})"
SCRIPT_DIR="$(dirname $SCRIPT)"
PWD=$(readlink -f `pwd`)

# Make sure the script is run from its proper path.
# OR Since we know the path, why don't we do it ourselves ?
if [ $PWD != $SCRIPT_DIR ]
then
    echo "Run the script from the hiphop-php directory like:"
    echo "cd $SCRIPT_DIR && ./$SCRIPT"
    exit 1
fi

# Identify the Distro
OS_TYPE=$(uname)
if [ "x$OS_TYPE" = "xLinux" ];then
    DISTRO_NAME=$(head -1 /etc/issue)
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

# Determine the CPUs irrespective of Travis Mode
CPUS=`cat /proc/cpuinfo | grep -E '^processor' | tail -1 | cut -d : -f 2`
CPUS=`expr ${CPUS} + 1`

# For travis
if [ "x${TRAVIS}" != "x" ]; then
  # Collect some stats for use in tuning build later on
  free
  echo "Travis Mode    : YES"
  echo "# CPUs         : ${CPUS}"
  echo ""
else
  echo "Travis Mode    : NO"
  echo "# CPUs         : ${CPUS}"
  echo ""
fi

# Place to save all the binaries/libraries/headers from the ext packages
export CMAKE_PREFIX_PATH=`/bin/pwd`/..

case $DISTRO in
    fedora)
        # install the actual dependencies
        sudo yum groupinstall "Fedora Packager" -y
        sudo yum install -y git wget make autoconf binutils-devel \
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
            ImageMagick-devel libxslt-devel

        # For patched stuff.
        git clone git://github.com/libevent/libevent.git --quiet &
        git clone git://github.com/bagder/curl.git --quiet &
        ;;
    ubuntu)
        # install python-software-properties before trying to add a PPA
        sudo apt-get -y update
        sudo apt-get install -y python-software-properties

        # install apt-fast to speed up later dependency installation
        sudo add-apt-repository -y ppa:apt-fast/stable
        sudo add-apt-repository -y ppa:mapnik/boost
        sudo apt-get -y update
        sudo apt-get -y install apt-fast

        # install the actual dependencies
        sudo apt-fast -y update
        sudo apt-fast -y install git-core cmake g++ libboost1.49-dev libmysqlclient-dev \
            libxml2-dev libmcrypt-dev libicu-dev openssl build-essential binutils-dev \
            libcap-dev libgd2-xpm-dev zlib1g-dev libtbb-dev libonig-dev libpcre3-dev \
            autoconf libtool libcurl4-openssl-dev libboost-regex1.49-dev libboost-system1.49-dev \
            libboost-program-options1.49-dev libboost-filesystem1.49-dev libboost-thread1.49-dev \
            wget memcached libreadline-dev libncurses-dev libmemcached-dev libbz2-dev \
            libc-client2007e-dev php5-mcrypt php5-imagick libgoogle-perftools-dev \
            libcloog-ppl0 libelf-dev libdwarf-dev libunwind7-dev subversion \
            libmagickwand-dev libxslt1-dev &

        git clone git://github.com/libevent/libevent.git --quiet &
        git clone git://github.com/bagder/curl.git --quiet &
        svn checkout http://google-glog.googlecode.com/svn/trunk/ google-glog --quiet &
        wget http://www.canonware.com/download/jemalloc/jemalloc-3.5.1.tar.bz2 --quiet &
        ;;
    *)
        echo "Unknown distribution. Please update packages in this section."
        exit 1
        ;;
esac

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

if [[ "x$DISTRO" == "xubuntu" ]];then
    # Leave this install till after the main parallel package install above
    # since it adds a non-12.04 package repo and we don't want to
    # pull EVERYTHING in, just the newer gcc compiler (and toolchain)
    GCC_VER=4.7
    if [ "x${TRAVIS}" != "x" ]; then
      GCC_VER=4.8
    fi
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get -y update
    sudo apt-get -y install gcc-${GCC_VER} g++-${GCC_VER}
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VER} 60 \
                                --slave /usr/bin/g++ g++ /usr/bin/g++-${GCC_VER}
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 40 \
                                --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
    sudo update-alternatives --set gcc /usr/bin/gcc-${GCC_VER}
fi

# libevent
cd libevent
git checkout release-1.4.14b-stable
cat ../hphp/third_party/libevent-1.4.14.fb-changes.diff | patch -p1
./autogen.sh
./configure --prefix=$CMAKE_PREFIX_PATH
make -j $CPUS
make install
cd ..

# curl
cd curl
./buildconf
./configure --prefix=$CMAKE_PREFIX_PATH
make -j $CPUS
make install
cd ..

if [[ "x$DISTRO" == "xubuntu" ]];then
    # glog
    cd google-glog
    ./configure --prefix=$CMAKE_PREFIX_PATH
    make -j $CPUS
    make install
    cd ..

    # jemaloc
    tar xjvf jemalloc-3.5.1.tar.bz2
    cd jemalloc-3.5.1
    ./configure --prefix=$CMAKE_PREFIX_PATH
    make -j $CPUS
    make install
    cd ..

    # cleanup
    rm -rf google-glog jemalloc-3.5.1.tar.bz2 jemalloc-3.5.1
fi

# cleanup
rm -rf libevent curl

# hphp
cmake .

# all set
echo "-------------------------------------------------------------------------"
echo "Done. Now run:"
echo "  CMAKE_PREFIX_PATH=\`pwd\`/.. make"
