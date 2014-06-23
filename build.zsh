
#set -x

source config.zsh

if [[ -n $excluded_modules ]]; then
  echo "-- exclude modules: $excluded_modules"
  export EXCLUDED_MODULES=$excluded_modules
fi

make_options=()

export GPFLAGS=-fpermissive

#export BUILD_TYPE=Debug
#export NONETWORK=1

if make $make_options all; then
  if $upload_after_build; then
    source upload.zsh
  fi
  if $burn_after_build; then
    source burn.zsh
  fi
fi
