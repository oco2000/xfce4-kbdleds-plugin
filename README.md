What is it?
===========

This is a keyboard leds plugin (based on sample plugin) for the Xfce panel.

![Screenshot](xfce4-kbdleds-plugin-screenshot.png)


Installation
============

You should have
- xfce4-dev-tools
- libxfce4panel-2.0-dev
- libxfce4ui-2-dev

Then run
```
./autogen.sh
./configure --prefix=/usr
sudo make install
```

On some distros you need to specify plugin directories by yourself like this
```
./configure --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu
```

If ScrollLock key does not work, you can enable it with this command
```
xmodmap -e 'add mod3 = Scroll_Lock'
```

[Source](https://askubuntu.com/questions/764405/scroll-lock-does-not-work)

Enjoy!
