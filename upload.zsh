
source config.zsh

if [[ -d $smoothie_upload_dir2 ]]; then
  cp -v $firmware_built $smoothie_upload_dir2/$firmware_file
  if [[ -e $smoothie_upload_dir2/$firmware_file ]]; then
    echo ok
  else
    echo FAILED
  fi
else
  if [[ -d $smoothie_upload_dir ]]; then
    cp -v $firmware_built $smoothie_upload_dir/$firmware_file
    if [[ -e $smoothie_upload_dir/$firmware_file ]]; then
      echo ok
    else
      echo FAILED
    fi
  else
    pmount $smoothie_disk_device                                &&
      cp -v $firmware_built $smoothie_upload_dir/$firmware_file
    pumount $smoothie_disk_device
    if [[ -e $smoothie_upload_dir/$firmware_file ]]; then
      echo ok
    else
      echo FAILED
    fi
  fi
fi
