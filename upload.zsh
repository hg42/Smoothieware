
source config.zsh

#pumount $smoothie_disk_device
pmount $smoothie_disk_device
cp -v $firmware_built $firmware_file
pumount $smoothie_disk_device
