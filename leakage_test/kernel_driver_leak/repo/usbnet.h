#include <stdint.h>
#include "skbuff.h"
#include <stdlib.h>

typedef uint32_t u32;
typedef u32 spinlock_t;
typedef u32 __u32;

struct wait_queue_head {
	spinlock_t		lock;
	struct list_head	head;
};
typedef struct wait_queue_head wait_queue_head_t;

#define struct_group_tagged(TAG, NAME, MEMBERS...) \
	__struct_group(TAG, NAME, /* no attrs */, MEMBERS)

struct sk_buff_head {
	/* These two members must be first to match sk_buff. */
	struct_group_tagged(sk_buff_list, list,
		struct sk_buff	*next;
		struct sk_buff	*prev;
	);

	__u32		qlen;
	spinlock_t	lock;
};

/* interface from usbnet core to each USB networking link we handle */
struct usbnet {
	/* housekeeping */
	struct usb_device	*udev;
	struct usb_interface	*intf;
	const struct driver_info *driver_info;
	const char		*driver_name;
	void			*driver_priv;
	wait_queue_head_t	wait;
//	struct mutex		phy_mutex;
	unsigned char		suspend_count;
	unsigned char		pkt_cnt, pkt_err;
	unsigned short		rx_qlen, tx_qlen;
	unsigned		can_dma_sg:1;

	/* i/o info: pipes etc */
	unsigned		in, out;
	struct usb_host_endpoint *status;
	unsigned		maxpacket;
//	struct timer_list	delay;
	const char		*padding_pkt;

	/* protocol/interface state */
	struct net_device	*net;
	int			msg_enable;
	unsigned long		data[5];
	u32			xid;
	u32			hard_mtu;	/* count any extra framing */
	size_t			rx_urb_size;	/* size for rx urbs */
//	struct mii_if_info	mii;
	long			rx_speed;	/* If MII not used */
	long			tx_speed;	/* If MII not used */
#		define SPEED_UNSET	-1

	/* various kinds of pending driver work */
	struct sk_buff_head	rxq;
	struct sk_buff_head	txq;
	struct sk_buff_head	done;
	struct sk_buff_head	rxq_pause;
	struct urb		*interrupt;
	unsigned		interrupt_count;
//	struct mutex		interrupt_mutex;
//	struct usb_anchor	deferred;
//	struct tasklet_struct	bh;

//	struct work_struct	kevent;
	unsigned long		flags;
#		define EVENT_TX_HALT	0
#		define EVENT_RX_HALT	1
#		define EVENT_RX_MEMORY	2
#		define EVENT_STS_SPLIT	3
#		define EVENT_LINK_RESET	4
#		define EVENT_RX_PAUSED	5
#		define EVENT_DEV_ASLEEP 6
#		define EVENT_DEV_OPEN	7
#		define EVENT_DEVICE_REPORT_IDLE	8
#		define EVENT_NO_RUNTIME_PM	9
#		define EVENT_RX_KILL	10
#		define EVENT_LINK_CHANGE	11
#		define EVENT_SET_RX_MODE	12
#		define EVENT_NO_IP_ALIGN	13
};
