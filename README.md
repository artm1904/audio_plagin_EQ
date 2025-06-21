1) Prerequsites - install this packages:

__For arch no need extra packeges (i use this system and with Wayland and Hyprland all works fine)__


__For Ubuntu need this:__

    - sudo apt-get install libxrandr-dev libx11-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev libcurl4-openssl-dev libxinerama-dev libgtk-3-dev  ibasound2-dev libwebkit2gtk-4.0-0

2) This repo contains 3 project 

Each projects have a few version Standalone (classic executable app), library version and VST3 plagin version.

- AudioPluginHost, standart JUCE app, where you can add you vst3 (and other) plagin and test is
- Play Audio, simple program (plagin) to play audio .wav (inclusive with AudioPluginHost to test audio plagin)
- EQ plagin, simple qualiser plagin