
source config.zsh

if make all; then
  if $upload_after_build; then
    source upload.zsh
  fi
  if $burn_after_build; then
    source burn.zsh
  fi
fi
