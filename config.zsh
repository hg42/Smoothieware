
SCRIPT_PATH=$0
SCRIPT_PATH=${SCRIPT_PATH%/*}
if [[ $SCRIPT_PATH == $0 ]]; then
  SCRIPT_PATH=$PWD
fi
cd $SCRIPT_PATH
SCRIPT_PATH=$PWD
GCC_DIR=$SCRIPT_PATH/gcc-arm-none-eabi
export PATH=$GCC_DIR/bin:$PATH
#export PATH=$GCC_DIR/arm-none-eabi/bin:$GCC_DIR/bin:$PATH
#export LIBRARY_PATH=$GCC_DIR/arm-none-eabi/lib/armv7-m:$GCC_DIR/arm-none-eabi/lib

#export CONSOLE=/dev/arduino
#export BAUD=1000000

# seems to be unused
#export LPC_DEPLOY='cp PROJECT.bin /media/MBED/ ; sync'
