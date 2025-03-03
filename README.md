# Terminal Radio
Listen to the radio in your terminal window

https://github.com/user-attachments/assets/4f006200-2b32-4a8e-87e9-97b3f72ca720

# Compiling
Make sure you have the curl development libraries and the mpv development libraries installed

- Ubuntu
```
sudo apt update
sudo apt install libcurl4-openssl-dev mpv
```

- Fedora
```
sudo dnf install curl-devel mpv-devel
```

- Mac
```
brew install curl
brew install mpv
```
Compile and run with make
```
make main
./main
```

# Keybinds
- Navigate with `hjkl`
- Control volume with `- and =`
- Pause/Unpause with `p`
- Go to top of list with `G`
- Select station with `enter`
- Enter/Leave filters window with `f`
- Close with `q` or `ctrl + c`

# Future Plans
- [ ] Make the code less spaghettified
- [ ] Fix UI bugs
- [ ] Add more keybinds for navigation
- [ ] Add favorited stations
