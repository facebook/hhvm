#/bin/sh
# called by src/

MK=${MAKE:-make}
J=-j

if [ -n "${USE_EMAKE}" ] ; then MK=${USE_EMAKE}; J=; fi

if [ $1 == "SHARED" ]; then
  SHARED=1 ${MK} -k -C runtime/tmp all $J
else
  COMPILE=1 ${MK} -k -C runtime/tmp all $J
  LINK=1 ${MK} -kj10 -C runtime/tmp all
fi
