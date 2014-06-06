
source config.zsh

make all                &&
  $upload_after_build   &&
    source upload.zsh
