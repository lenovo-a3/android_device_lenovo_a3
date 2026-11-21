#!/system/bin/sh
# Bring-up log catcher (init.a3.debug.rc). Writes to /cache/bootlog, which our
# recovery can read: boot recovery, then
#   adb -s 0123456789ABCDEF pull /cache/bootlog
# Snapshots every 15 s for the first 3 minutes; logcat runs as its own service.
out=/cache/bootlog
mkdir -p $out
for i in 1 2 3 4 5 6 7 8 9 10 11 12; do
    dmesg > $out/dmesg.txt
    getprop > $out/getprop.txt
    ps -t > $out/ps.txt
    cat /proc/mounts > $out/mounts.txt
    ls -l /dev /dev/block /dev/graphics > $out/dev.txt 2>&1
    lsmod > $out/lsmod.txt 2>&1
    for f in state functions enable idVendor idProduct; do
        echo "$f: $(cat /sys/class/android_usb/android0/$f 2>&1)"
    done > $out/android_usb.txt
    echo "snapshot $i at $(cat /proc/uptime)" > $out/done.txt
    sync
    sleep 15
done
