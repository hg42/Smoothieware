
SCRIPT_PATH=$0
SCRIPT_PATH=${SCRIPT_PATH%/*}
cd $SCRIPT_PATH
SCRIPT_PATH=$PWD
export PATH=$SCRIPT_PATH/gcc-arm-none-eabi/bin:$PATH

#export CONSOLE=/dev/arduino
#export BAUD=1000000

# seems to be unused
#export LPC_DEPLOY='cp PROJECT.bin /media/MBED/ ; sync'
