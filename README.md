# ppcplayer

Proof of concept video player for Mac OS 9 that uses modern ffmpeg and supports more modern formats. Why? We don't ask those questions.

Only does video, and has a known bug where mp4s don't play (but other video formats with the same codec do play) on weaker machines. [There was an attempt](https://github.com/IoIxD/ppcplayer/tree/with-audio-attempts) to get audio working, but I spent literal days on it and eventually gave up because anything that ffmpeg will play nicely with is either too new or too hard.

[See the supported codecs](https://github.com/IoIxD/ppcplayer/wiki/Supported-Codecs).

To download, see the releases sidebar. To build from source, just get Retro68 and use `./build.sh`.
