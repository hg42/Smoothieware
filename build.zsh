
source config.zsh

if [[ -n $excluded_modules ]]; then
  echo "-- exclude modules: $excluded_modules"
  excluded_modules="EXCLUDED_MODULES=$excluded_modules"
fi

if make $excluded_modules all; then
  if $upload_after_build; then
    source upload.zsh
  fi
  if $burn_after_build; then
    source burn.zsh
  fi
fi
