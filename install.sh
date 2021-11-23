#!/bin/sh

PROJECT_OWNER=GaZaTu
PROJECT_REPO=x11-emoji-picker
PROJECT_NAME=$PROJECT_REPO

RELEASE="latest"

while getopts ":r:" flag
do
  case $flag in
    r) RELEASE="$OPTARG";;
  esac
done

echo "Installing $PROJECT_NAME release: $RELEASE..."

DISTRO_ID=$(grep "^ID=" "/etc/os-release" | sed "s/ID=//" | sed "s/\"//g")
DISTRO_VERSION_ID=$(grep "^VERSION_ID=" "/etc/os-release" | sed "s/VERSION_ID=//" | sed "s/\"//g")

PACKAGE_MANAGER=$(
  case "$DISTRO_ID" in
    "ubuntu"* | "debian"*) echo "apt";;
    "opensuse"*) echo "zypper";;
    "fedora"*) echo "dnf";;
  esac
)
PACKAGE_MANAGER_INSTALL_COMMAND=$(
  case "$PACKAGE_MANAGER" in
    "apt") echo "install -y";;
    "zypper") echo "install -y --allow-unsigned-rpm";;
    "dnf") echo "install -y --nogpgcheck";;
  esac
)

if ! command -v jq > /dev/null;
then
  if [ -z "$PACKAGE_MANAGER" ];
  then
    echo "jq not found, and no known package manager found to install it either"
    exit 1
  fi

  echo ""
  echo "jq not found, installing through package manager..."
  echo "> sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND jq"
  sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND jq
fi

echo ""
echo "Check github for release artifacts..."
RELEASE_ID=$(echo $RELEASE | sed "s/\//-/")
RELEASE_JSON="/tmp/$PROJECT_REPO-$RELEASE_ID.json"
wget -nv "https://api.github.com/repos/$PROJECT_OWNER/$PROJECT_REPO/releases/$RELEASE" -O "$RELEASE_JSON"

echo ""
echo "Find artifact for $DISTRO_ID $DISTRO_VERSION_ID"
URL=$(jq -r ".assets | .[] | .browser_download_url | select(. | (test(\"$DISTRO_ID\") and test(\"$DISTRO_VERSION_ID\")))" "$RELEASE_JSON")
ARTIFACT_IS_APPIMAGE=false

if [ -z "$URL" ];
then
  echo "Matching artifact not found; Find .AppImage instead"
  URL=$(jq -r ".assets | .[] | .browser_download_url | select(. | (test(\".AppImage\")))" "$RELEASE_JSON")
  ARTIFACT_IS_APPIMAGE=true
fi

if [ -z "$URL" ];
then
  echo "No artifact found; something went wrong"
  exit 1
fi

ARTIFACT_NAME=$(basename "$URL")
ARTIFACT_PATH="/tmp/$ARTIFACT_NAME"

EXECUTABLE_NAME="emoji-picker"

APPIMAGE_NAME="$EXECUTABLE_NAME.AppImage"
APPIMAGE_INSTALL_PATH="/usr/bin/$APPIMAGE_NAME"

echo ""
echo "Downloading artifact..."
wget -q --show-progress "$URL" -O "$ARTIFACT_PATH"

if $ARTIFACT_IS_APPIMAGE
then
  if command -v $EXECUTABLE_NAME > /dev/null;
  then
    echo ""
    echo "Abort: do not override $EXECUTABLE_NAME with .AppImage"
    exit 2
  fi

  EXECUTABLE_NAME="$APPIMAGE_NAME"

  echo ""
  echo "Move artifact and make it executable"
  echo "> sudo cp $ARTIFACT_PATH $APPIMAGE_INSTALL_PATH"
  sudo cp "$ARTIFACT_PATH" "$APPIMAGE_INSTALL_PATH"
  echo "> sudo chmod +x $APPIMAGE_INSTALL_PATH"
  sudo chmod +x "$APPIMAGE_INSTALL_PATH"
else
  if command -v $APPIMAGE_NAME > /dev/null;
  then
    echo ""
    echo "Remove existing .AppImage"
    echo "> sudo rm $APPIMAGE_INSTALL_PATH"
    sudo rm "$APPIMAGE_INSTALL_PATH"
  fi

  echo ""
  echo "Install artifact using package manager"
  echo "> sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND $ARTIFACT_PATH"
  sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND "$ARTIFACT_PATH"
fi

if echo "$XDG_CURRENT_DESKTOP" | grep -iqF "gnome";
then
  REGISTER_KEYBIND=true
  echo ""
  read -p "Install keyboard shortcut (Super+.)? (Y/n): " choice
  case "$choice" in
    n|N) REGISTER_KEYBIND=false;;
    y|Y) REGISTER_KEYBIND=true;;
  esac

  if $REGISTER_KEYBIND
  then
    # Adding the keyboard shortcut
    # Fetching all custom keyboard shortcuts
    CUSTOM_KEYBINDINGS_LIST=$(gsettings get org.gnome.settings-daemon.plugins.media-keys custom-keybindings)
    if ! echo "$CUSTOM_KEYBINDINGS_LIST" | grep -q "$PROJECT_REPO"
    then
      if [ "$CUSTOM_KEYBINDINGS_LIST" = "@as []" ] # creating new list if not exists
      then
        CUSTOM_KEYBINDINGS_LIST="['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/$PROJECT_REPO/']"
      else # if already existing, adding our new element
        CUSTOM_KEYBINDINGS_LIST=$(echo "$CUSTOM_KEYBINDINGS_LIST" | sed 's/\[//' | sed 's/\]//')
        CUSTOM_KEYBINDINGS_LIST="[$CUSTOM_KEYBINDINGS_LIST, '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/$PROJECT_REPO/']"
      fi

      gsettings set org.gnome.settings-daemon.plugins.media-keys custom-keybindings "$CUSTOM_KEYBINDINGS_LIST" # update the list
      gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/$PROJECT_REPO/ name "$PROJECT_NAME" # set name
      gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/$PROJECT_REPO/ command "$EXECUTABLE_NAME" # set command
      gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/$PROJECT_REPO/ binding "<Super>period" # set shortcut
      echo "Keyboard shortcut added!"
    else
      echo "Keyboard shortcut already added."
    fi
  fi
fi

if echo "$XDG_CURRENT_DESKTOP" | grep -iqF "i3";
then
  REGISTER_KEYBIND=true
  echo ""
  read -p "Install keyboard shortcut (Super+.)? (Y/n): " choice
  case "$choice" in
    n|N) REGISTER_KEYBIND=false;;
    y|Y) REGISTER_KEYBIND=true;;
  esac

  if $REGISTER_KEYBIND
  then
    I3_CONFIG=~/.config/i3/config

    # Adding the keyboard shortcut
    if ! cat $I3_CONFIG | grep -q "$EXECUTABLE_NAME";
    then
      echo "
bindsym \$mod+period exec --no-startup-id $EXECUTABLE_NAME
" >> $I3_CONFIG
    else
      echo "Keyboard shortcut already added."
    fi
  fi
fi

echo ""
echo "$PROJECT_NAME installed! 🎉"
