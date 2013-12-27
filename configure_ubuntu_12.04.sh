#########################################
# 
# Install all the dependencies for HipHop
#
#########################################

SCRIPT_NAME='./configure_ubuntu_12.04.sh'
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

# install python-software-properties before trying to add a PPA
sudo apt-get -y update
sudo apt-get install -y python-software-properties

# install apt-fast to speed up later dependency installation
sudo add-apt-repository -y ppa:apt-fast/stable
sudo apt-get -y update
sudo apt-get -y install apt-fast

# install the actual dependencies
sudo apt-fast -y update
sudo apt-fast -y install git-core cmake g++ libboost1.48-dev libmysqlclient-dev \
  libxml2-dev libmcrypt-dev libicu-dev openssl build-essential binutils-dev \
  libcap-dev libgd2-xpm-dev zlib1g-dev libtbb-dev libonig-dev libpcre3-dev \
  autoconf libtool libcurl4-openssl-dev libboost-regex1.48-dev libboost-system1.48-dev \
  libboost-program-options1.48-dev libboost-filesystem1.48-dev libboost-thread1.48-dev \
  wget memcached libreadline-dev libncurses-dev libmemcached-dev libbz2-dev \
  libc-client2007e-dev php5-mcrypt php5-imagick libgoogle-perftools-dev \
  libcloog-ppl0 libelf-dev libdwarf-dev libunwind7-dev subversion &

git clone git://github.com/libevent/libevent.git --quiet &
git clone git://github.com/bagder/curl.git --quiet &
svn checkout http://google-glog.googlecode.com/svn/trunk/ google-glog --quiet &
wget http://www.canonware.com/download/jemalloc/jemalloc-3.0.0.tar.bz2 --quiet &

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

# glog
cd google-glog
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# jemaloc
tar xjvf jemalloc-3.0.0.tar.bz2
cd jemalloc-3.0.0
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# cleanup
rm -rf libevent curl google-glog jemalloc-3.0.0.tar.bz2 jemalloc-3.0.0

# hphp
cmake .

echo "-------------------------------------------------------------------------"
echo "Done. Now run:"
echo "  CMAKE_PREFIX_PATH=\`pwd\`/.. make"
