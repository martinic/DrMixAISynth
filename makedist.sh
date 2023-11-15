#!/bin/bash

bundle=Doctor\ Mix\ AI\ Synth

if [ $# -eq 1 ] && [ "$1" == "--clean" ]
then
	rm -rf build
	exit
fi

if [ $# -ne 0 ]
then
	echo "Usage: $0 [--clean]"
	exit 1
fi

rm -rf ~/"Library/Audio/Plug-Ins/Components/$bundle.component/" || exit
rm -rf ~/"Library/Audio/Plug-Ins/CLAP/$bundle.clap/" || exit
rm -rf ~/"Library/Audio/Plug-Ins/VST/$bundle.vst/" || exit
rm -rf ~/"Library/Audio/Plug-Ins/VST3/$bundle.vst3/" || exit

xcodebuild clean || exit

for target in AU CLAP VST2 VST3
do
	xcodebuild -configuration Release ndebug=NDEBUG GCC_WARN_ABOUT_DEPRECATED_FUNCTIONS=NO -target $target install || exit
done
