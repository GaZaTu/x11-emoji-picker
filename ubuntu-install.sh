echo "Ask github for the latest version of x11-emoji-picker..."
wget -q https://api.github.com/repos/GaZaTu/x11-emoji-picker/releases/latest

# Install jq if not already installed
jqInstalledNow=false

if ( ! which jq &> /dev/null )
then
    echo ""
    echo "jq not found, installing through APT..."
    sudo apt update
    sudo apt install jq -y
    jqInstalledNow=true
fi


echo "Fetch the link from the github json file..."
jq -r '.assets | .[] | .browser_download_url | select(. | test("ubuntu"))' latest > url

if $jqInstalledNow
then
    echo ""
    echo "Remove jq because it was installed for this event..."
    sudo apt autoremove jq -y
fi

rm latest
echo "Downloading package..."
wget -q --show-progress -i url -O x11-emoji-picker.deb
rm url

echo "Installing the emoji picker..."
sudo apt install ./x11-emoji-picker.deb -y

# Deleting the .deb
rm x11-emoji-picker.deb

installKeyboardShortcut=true
echo ""
read -p "Install keyboard shortcut (Super + ;)? (Y/N): " confirm && [[ $confirm == [yY] ]] || installKeyboardShortcut=false

if $installKeyboardShortcut
then
    # Adding the keyboard shortcut
    # Fetching all customs keyboard shortcuts
    CUSTOM_KEYBINDINGS_LIST=$(gsettings get org.gnome.settings-daemon.plugins.media-keys custom-keybindings)
    if [[ $CUSTOM_KEYBINDINGS_LIST == "@as []" ]] # creating new list if not exists
    then
        CUSTOM_KEYBINDINGS_LIST="['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/x11-emoji-picker/']"
    else # if already existing, adding our new element
        CUSTOM_KEYBINDINGS_LIST="${CUSTOM_KEYBINDINGS_LIST::-1}, '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/x11-emoji-picker/']"
    fi
    gsettings set org.gnome.settings-daemon.plugins.media-keys custom-keybindings "$CUSTOM_KEYBINDINGS_LIST" # update the list
    gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/x11-emoji-picker/ name 'Emoji Picker' # set name
    gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/x11-emoji-picker/ command 'emoji-picker' # set command
    gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/x11-emoji-picker/ binding '<Super>semicolon' # set shortcut
    echo "Keyboard shortcut added!"
fi

echo ""
echo "Emoji picker installed!"
