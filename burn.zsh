
source config.zsh

dfu-util --download $firmware_built --device 1d50:6015 --reset
