
source config.zsh

make upload
  # = dfu-util --download $firmware_built --device 1d50:6015 --reset

sleep 12
ll /dev/ttyA* /dev/disk/by-label/SMOOTH*
