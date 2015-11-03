#!/bin/bash

#set -x
if [ -f /tmp/omxplayerdbus.$USER ]
then
	OMXPLAYER_DBUS_ADDR="/tmp/omxplayerdbus.$USER"
	OMXPLAYER_DBUS_PID="/tmp/omxplayerdbus.$USER.pid"
else
	OMXPLAYER_DBUS_ADDR="/tmp/omxplayerdbus"
	OMXPLAYER_DBUS_PID="/tmp/omxplayerdbus.pid"
fi
export DBUS_SESSION_BUS_ADDRESS=`cat $OMXPLAYER_DBUS_ADDR`
export DBUS_SESSION_BUS_PID=`cat $OMXPLAYER_DBUS_PID`

[ -z "$DBUS_SESSION_BUS_ADDRESS" ] && { echo "Must have DBUS_SESSION_BUS_ADDRESS" >&2; exit 1; }

case $2 in
status)
	duration=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Duration`
	[ $? -ne 0 ] && exit 1
	duration="$(awk '{print $2}' <<< "$duration")"

	position=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Position`
	[ $? -ne 0 ] && exit 1
	position="$(awk '{print $2}' <<< "$position")"

	playstatus=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.PlaybackStatus`
	[ $? -ne 0 ] && exit 1
	playstatus="$(sed 's/^ *//;s/ *$//;' <<< "$playstatus")"

	paused="true"
	[ "$playstatus" == "Playing" ] && paused="false"
	echo "Duration: $duration"
	echo "Position: $position"
	echo "Paused: $paused"
	;;

canseek)
	canseek=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.CanSeek`
	[ $? -ne 0 ] && exit 1
	canseek="$(awk '{print $2}' <<< "$canseek")"
	echo $canseek
	;;

duration)
	duration=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Duration`
	[ $? -ne 0 ] && exit 1
	duration="$(awk '{print $2}' <<< "$duration")"
	echo "$duration"
	;;

aspect)
	aspect=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Aspect`
	[ $? -ne 0 ] && exit 1
	aspect="$(awk '{print $2}' <<< "$aspect")"
	echo "$aspect"
	;;

volume)
	volume=`dbus-send --print-reply=literal --session --reply-timeout=500 --dest=$1 /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Volume ${3:+double:}$3`
	[ $? -ne 0 ] && exit 1
	volume="$(awk '{print $2}' <<< "$volume")"
	echo "Volume: $volume"
	;;

pause)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:16 >/dev/null
	;;

stop)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:15 >/dev/null
	;;

seek)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Seek int64:$3 >/dev/null
	;;

setposition)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.SetPosition objpath:/not/used int64:$3 >/dev/null
	;;

setalpha)
	dbus-send --print-reply=literal --session  --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.SetAlpha objpath:/not/used int64:$3 >/dev/null
	;;

setvideopos)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.VideoPos objpath:/not/used string:"$3 $4 $5 $6" >/dev/null
	;;

hidevideo)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:28 >/dev/null
	;;

unhidevideo)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:29 >/dev/null
	;;

volumeup)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:18 >/dev/null
	;;

volumedown)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:17 >/dev/null
	;;

togglesubtitles)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:12 >/dev/null
	;;
	
hidesubtitles)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:30 >/dev/null
	;;

showsubtitles)
	dbus-send --print-reply=literal --session --dest=$1 /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Action int32:31 >/dev/null
	;;

*)
	echo "usage: $0 destination status|duration|aspect|canseek|pause|stop|seek|volumeup|volumedown|setposition [position in microseconds]|hidevideo|unhidevideo|togglesubtitles|hidesubtitles|showsubtitles|setvideopos [x1 y1 x2 y2]|setalpha [alpha (0..255)]" >&2
	exit 1
	;;
esac
