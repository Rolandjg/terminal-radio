# Terminal Radio
Listen to the radio in your terminal window

# Compiling
Make sure you have the curl development libraries and the mpv development libraries installed

Ubuntu
```
sudo apt update
sudo apt install libcurl4-openssl-dev mpv
```

Fedora
```
sudo dnf install curl-devel mpv-devel
```

Mac
```
brew install curl
brew install mpv
```

# Keybinds
Navigate with `hjkl`
Control volume with `- and =`
Pause/Unpause with `p`
Go to top of list with `G`
Select station with `enter`
Enter/Leave filters window with `f`
Close with `q` or `ctrl + c`

# Future Plans
- [ ] Make the code less spaghettified
- [ ] Fix UI bugs
- [ ] Add more keybinds for navigation
- [ ] Add favorited stations
