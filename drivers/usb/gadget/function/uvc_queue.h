#ifndef _UVC_QUEUE_H_
#define _UVC_QUEUE_H_

#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/videodev2.h>
#include <media/videobuf2-v4l2.h>

/* Maximum frame size in bytes, for sanity checking. */
#define UVC_MAX_FRAME_SIZE	(16*1024*1024)
/* Maximum number of video buffers. */
#define UVC_MAX_VIDEO_BUFFERS	32
#define UVC_MAX_REQ_SG_LIST_NUM 12

/* ------------------------------------------------------------------------
 * Structures.
 */

enum uvc_buffer_state {
	UVC_BUF_STATE_IDLE	= 0,
	UVC_BUF_STATE_QUEUED	= 1,
	UVC_BUF_STATE_ACTIVE	= 2,
	UVC_BUF_STATE_DONE	= 3,
	UVC_BUF_STATE_ERROR	= 4,
};

struct uvc_buffer {
	struct vb2_v4l2_buffer buf;
	struct list_head queue;

	enum uvc_buffer_state state;
	void *mem;
#ifdef CONFIG_USB_WEBCAM_UVC_SUPPORT_SG_TABLE
	struct sg_table sgt;
#endif
	unsigned int length;
	unsigned int bytesused;
#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	bool bFrameEnd;
#endif
};

#define UVC_QUEUE_DISCONNECTED		(1 << 0)
#define UVC_QUEUE_DROP_INCOMPLETE	(1 << 1)
#define UVC_QUEUE_PAUSED		(1 << 2)

struct uvc_video_queue {
	struct vb2_queue queue;

	unsigned int flags;
	__u32 sequence;

	unsigned int buf_used;
#ifdef CONFIG_USB_WEBCAM_UVC_SUPPORT_SG_TABLE
	struct scatterlist *cur_sg;
#endif
#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	bool bFrameEnd;
#endif
#if defined(CONFIG_UVC_STREAM_ERR_SUPPORT)
	/* Use to inform host to drop the receiving frame(s) */
	__u8 bXferFlag;
	#define FLAG_UVC_XFER_OK		0x00
	#define FLAG_UVC_XFER_ERR		0x01 // flag to indicate data loss in xfer
	#define FLAG_UVC_EVENT_KEYFRAME	0x40 // flag to send forceIDR event
	#define FLAG_UVC_WAIT_KEYFRAME	0x80 // flag to keep drop frames unitl I-frame queue
#endif
	spinlock_t irqlock;	/* Protects flags and irqqueue */
	struct list_head irqqueue;
};

static inline int uvc_queue_streaming(struct uvc_video_queue *queue)
{
	return vb2_is_streaming(&queue->queue);
}

int uvcg_queue_init(struct uvc_video_queue *queue, enum v4l2_buf_type type,
		    struct mutex *lock);

void uvcg_free_buffers(struct uvc_video_queue *queue);

int uvcg_alloc_buffers(struct uvc_video_queue *queue,
		       struct v4l2_requestbuffers *rb);

int uvcg_query_buffer(struct uvc_video_queue *queue, struct v4l2_buffer *buf);

int uvcg_queue_buffer(struct uvc_video_queue *queue, struct v4l2_buffer *buf);

int uvcg_dequeue_buffer(struct uvc_video_queue *queue,
			struct v4l2_buffer *buf, int nonblocking);

unsigned int uvcg_queue_poll(struct uvc_video_queue *queue,
			     struct file *file, poll_table *wait);

int uvcg_queue_mmap(struct uvc_video_queue *queue, struct vm_area_struct *vma);

#ifndef CONFIG_MMU
unsigned long uvcg_queue_get_unmapped_area(struct uvc_video_queue *queue,
					   unsigned long pgoff);
#endif /* CONFIG_MMU */

void uvcg_queue_cancel(struct uvc_video_queue *queue, int disconnect);

int uvcg_queue_enable(struct uvc_video_queue *queue, int enable);

struct uvc_buffer *uvcg_queue_next_buffer(struct uvc_video_queue *queue,
					  struct uvc_buffer *buf);

struct uvc_buffer *uvcg_queue_head(struct uvc_video_queue *queue);

#ifdef CONFIG_USB_WEBCAM_UVC_SUPPORT_SG_TABLE

void uvcg_complete_sg(struct uvc_video_queue *queue);

struct uvc_buffer *uvcg_prepare_sg(struct uvc_video_queue *queue);

#endif

#endif /* __KERNEL__ */

#endif /* _UVC_QUEUE_H_ */

