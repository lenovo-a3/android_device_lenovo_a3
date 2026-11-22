#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#define BTM_DEF_LOCAL_NAME   "Lenovo IdeaTab A3000-H"
#define BTA_AV_MAX_A2DP_MTU  1788

/*
 * Use the stock eager_reader for the H4 stream, not CAF's hci_reader.
 * hci_reader reads into a fixed 1026-byte buffer and only rewinds it when a
 * packet ends exactly at the end of the buffered data; under A2DP load it
 * fills up and then issues read(fd, p, 0). MTK's /dev/stpbt (BT_read) treats
 * a 0-byte receive as "no data" and waits inside the kernel forever, so
 * hci_thread hangs at 100% CPU and all HCI traffic stops.
 */
#define REMOVE_EAGER_THREADS FALSE

#endif
